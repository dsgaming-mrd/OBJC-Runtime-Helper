//
// Created by DS Gaming - Mr D on 23/05/2026.
//

#pragma once

#import <Foundation/Foundation.h>
#import <objc/runtime.h>
#import <objc/message.h>

#include <stdint.h>
#include <dlfcn.h>

namespace RuntimeHelper {
    static inline Class GetClass(const char *name) {
        return (name && *name) ? objc_getClass(name) : Nil;
    }

    static inline SEL GetSelector(const char *name) {
        return (name && *name) ? sel_registerName(name) : NULL;
    }

    static inline bool IsObject(void *obj) {
        return obj && object_getClass((__bridge id)obj) != nullptr;
    }

    static inline const char *GetClassName(void *obj) {
        if (!obj) return nullptr;
    
        id o = (__bridge id)obj;
        Class cls = object_getClass(o);
        if (!cls) return nullptr;
    
        return class_getName(cls);
    }

    static inline bool HasInstanceMethod(const char *clsName, const char *selName) {
        Class cls = GetClass(clsName);
        SEL sel = GetSelector(selName);
        if (!cls || !sel) return false;

        return class_getInstanceMethod(cls, sel) != NULL;
    }

    static inline bool HasClassMethod(const char *clsName, const char *selName) {
        Class cls = GetClass(clsName);
        SEL sel = GetSelector(selName);
        if (!cls || !sel) return false;

        return class_getClassMethod(cls, sel) != NULL;
    }

    static inline Ivar GetIvarData(const char *clsName, const char *ivarName) {
        Class cls = GetClass(clsName);
        if (!cls || !ivarName || !*ivarName) return NULL;

        return class_getInstanceVariable(cls, ivarName);
    }

    static inline Method GetMethodData(const char *clsName, const char *selName, bool isClassMethod = false) {
        Class cls = GetClass(clsName);
        SEL sel = GetSelector(selName);
        if (!cls || !sel) return NULL;

        return isClassMethod ? class_getClassMethod(cls, sel) : class_getInstanceMethod(cls, sel);
    }

    static inline IMP GetMethodIMP(const char *clsName, const char *selName, bool isClassMethod = false) {
        Method method = GetMethodData(clsName, selName, isClassMethod);
        return method ? method_getImplementation(method) : NULL;
    }
}

namespace OBJCRuntime {
    template<typename T>
    static inline T Read(uintptr_t addr) {
        return *(T *)addr;
    }

    template<typename T>
    static inline void Write(uintptr_t addr, T value) {
        *(T *)addr = value;
    }

    static inline uintptr_t GetIvarOffset(const char *clsName, const char *ivarName) {
        Ivar ivar = RuntimeHelper::GetIvarData(clsName, ivarName);
        return ivar ? ivar_getOffset(ivar) : 0;
    }

    template<typename T>
    static inline T GetIvar(void *obj, const char *clsName, const char *ivarName) {
        if (!RuntimeHelper::IsObject(obj) || !clsName || !ivarName) return (T)0;

        uintptr_t off = GetIvarOffset(clsName, ivarName);
        if (!off) return (T)0;

        return *(T *)((uintptr_t)obj + off);
    }

    template<typename T>
    static inline void SetIvar(void *obj, const char *clsName, const char *ivarName, T value) {
        if (!RuntimeHelper::IsObject(obj) || !clsName || !ivarName) return;

        uintptr_t off = GetIvarOffset(clsName, ivarName);
        if (!off) return;

        *(T *)((uintptr_t)obj + off) = value;
    }

    template<typename T, typename... Args>
    static inline T GetInstanceMethod(void *obj, const char *clsName, const char *selName, Args... args) {
        if (!RuntimeHelper::IsObject(obj) || !clsName || !selName) return (T)0;

        Class cls = RuntimeHelper::GetClass(clsName);
        if (!cls) return (T)0;

        id target = (__bridge id)obj;
        SEL sel = RuntimeHelper::GetSelector(selName);

        if (!sel || ![target respondsToSelector:sel])
            return (T)0;

        using MsgSend = T (*)(id, SEL, ...);
        return ((MsgSend)objc_msgSend)(target, sel, args...);
    }

    template<typename T, typename... Args>
    static inline T GetClassMethod(const char *clsName, const char *selName, Args... args) {
        if (!clsName || !selName) return (T)0;

        Class cls = RuntimeHelper::GetClass(clsName);
        if (!cls) return (T)0;

        SEL sel = RuntimeHelper::GetSelector(selName);

        if (!sel || ![cls respondsToSelector:sel])
            return (T)0;

        using MsgSend = T (*)(id, SEL, ...);
        return ((MsgSend)objc_msgSend)((id)cls, sel, args...);
    }

    static inline uintptr_t GetInstanceMethodRealAddress(const char *clsName, const char *selName) {
        return (uintptr_t)RuntimeHelper::GetMethodIMP(clsName, selName, false);
    }

    static inline uintptr_t GetClassMethodRealAddress(const char *clsName, const char *selName) {
        return (uintptr_t)RuntimeHelper::GetMethodIMP(clsName, selName, true);
    }

    static inline uintptr_t GetInstanceMethodOffsetRVA(const char *clsName, const char *selName) {
        uintptr_t imp = GetInstanceMethodRealAddress(clsName, selName);
        if (!imp) return 0;
    
        Dl_info info;
        if (!dladdr((void *)imp, &info))
            return 0;
    
        return (uintptr_t)imp - (uintptr_t)info.dli_fbase;
    }

    static inline uintptr_t GetClassMethodOffsetRVA(const char *clsName, const char *selName) {
        uintptr_t imp = GetClassMethodRealAddress(clsName, selName);
        if (!imp) return 0;
    
        Dl_info info;
        if (!dladdr((void *)imp, &info))
            return 0;
    
        return (uintptr_t)imp - (uintptr_t)info.dli_fbase;
    }

    static inline uintptr_t GetInstanceMethodVA(const char *clsName, const char *selName) {
        uintptr_t imp = GetInstanceMethodRealAddress(clsName, selName);
        if (!imp) return 0;
    
        Dl_info info;
        if (!dladdr((void *)imp, &info))
            return 0;
    
        uintptr_t slide = 0;
        uint32_t imageCount = _dyld_image_count();
        for (uint32_t i = 0; i < imageCount; i++) {
            if (strcmp(_dyld_get_image_name(i), info.dli_fname) == 0) {
                slide = _dyld_get_image_vmaddr_slide(i);
                break;
            }
        }
    
        return imp - slide;
    }

    static inline uintptr_t GetClassMethodVA(const char *clsName, const char *selName) {
        uintptr_t imp = GetClassMethodRealAddress(clsName, selName);
        if (!imp) return 0;
    
        Dl_info info;
        if (!dladdr((void *)imp, &info))
            return 0;
    
        uintptr_t slide = 0;
        uint32_t imageCount = _dyld_image_count();
        for (uint32_t i = 0; i < imageCount; i++) {
            if (strcmp(_dyld_get_image_name(i), info.dli_fname) == 0) {
                slide = _dyld_get_image_vmaddr_slide(i);
                break;
            }
        }
    
        return imp - slide;
    }

    static inline bool HookInstanceMethod(const char *clsName, const char *selName, void *hook, void **orig = nullptr) {
        Method method = RuntimeHelper::GetMethodData(clsName, selName, false);
        if (!method || !hook) return false;
    
        IMP old = method_getImplementation(method);
        if (orig) {
            *orig = (void *)old;
        }
    
        method_setImplementation(method, (IMP)hook);
        return true;
    }
    
    static inline bool HookClassMethod(const char *clsName, const char *selName, void *hook, void **orig = nullptr) {
        Method method = RuntimeHelper::GetMethodData(clsName, selName, true);
        if (!method || !hook) return false;
    
        IMP old = method_getImplementation(method);
        if (orig) {
            *orig = (void *)old;
        }
    
        method_setImplementation(method, (IMP)hook);
        return true;
    }
}
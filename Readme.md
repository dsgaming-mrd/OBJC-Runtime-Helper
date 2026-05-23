# OBJC Runtime Helper

**Platform:** iOS / macOS / Theos / Linux  
**Language:** C++ / Objective-C++  

---

# 📌 Introduction

OBJCRuntime is a lightweight Objective-C runtime helper that provides direct access to runtime operations such as:

- Class / selector lookup
- Method invocation
- Ivar get/set via offset
- Method introspection (`IMP` / offset resolution)
- Raw memory read/write

### 💡 Inspiration & Design Goal
This utility was heavily inspired by the **Unity IL2CPP API** structure. By mirroring those familiar naming conventions and paradigms, OBJCRuntime simplifies syntax, reduces boilerplate code, and streamlines native runtime manipulation for iOS/macOS applications.

---

# 🚀 CORE CONCEPT

This utility operates directly on Objective-C runtime APIs:

- `objc_msgSend`
- `class_getInstanceVariable`
- `ivar_getOffset`
- `method_getImplementation`
- `dladdr` (for address resolution)
- Direct pointer arithmetic using `uintptr_t` for memory access

---

# 🧠 MEMORY ACCESS (RAW ADDRESS READ / WRITE)

## 🔹 Read Memory Address

Read a value directly from a raw memory address (`uintptr_t`).

### Syntax
```cpp
OBJCRuntime::Read<Type>(address);
```

### Examples
```cpp
uintptr_t a = ...;
uintptr_t b = ...;
uintptr_t c = ...;

int A = OBJCRuntime::Read<int>(a);
float B = OBJCRuntime::Read<float>(b);
bool C = OBJCRuntime::Read<bool>(c);
```

---

## 🔹 Write Memory Address

Write a value directly into a raw memory address.

### Syntax
```cpp
OBJCRuntime::Write<Type>(address, value);
```

### Examples
```cpp
uintptr_t a = ...;
uintptr_t b = ...;
uintptr_t c = ...;

OBJCRuntime::Write<int>(a, 999);
OBJCRuntime::Write<float>(b, 1.5f);
OBJCRuntime::Write<bool>(c, true);
```

---

# 🛠️ RUNTIME HELPER SYSTEM (INTERNAL UTILS)

Internal utility functions used to quickly inspect or safely retrieve basic runtime info.

---

### 🔹 GetClass
Safely look up and return a Class object by its string name.
```cpp
Class playerClass = RuntimeHelper::GetClass("Player");
```

### 🔹 GetSelector
Register and return an Objective-C selector by its string name.
```cpp
SEL updateSel = RuntimeHelper::GetSelector("update");
```

### 🔹 IsObject
Check if a raw pointer points to a valid Objective-C object.
```cpp
bool isObj = RuntimeHelper::IsObject(obj);
```

### 🔹 GetClassName
Retrieve the class name of a given object as a string.
```cpp
const char *name = RuntimeHelper::GetClassName(obj);
```

### 🔹 HasInstanceMethod
Check if a specific class implements a given instance method.
```cpp
bool hasMethod = RuntimeHelper::HasInstanceMethod("Player", "getHP");
```

### 🔹 HasClassMethod
Check if a specific class implements a given static class method.
```cpp
bool hasClassMethod = RuntimeHelper::HasClassMethod("GameManager", "shared");
```

### 🔹 GetIvarData
Retrieve the direct `Ivar` metadata pointer for a specific variable inside a class.
```cpp
Ivar hpIvar = RuntimeHelper::GetIvarData("Player", "m_hp");
```

### 🔹 GetMethodData
Retrieve the raw `Method` structural pointer from the runtime. Pass `true` for static class methods, or `false` for standard instance methods. (Mainly used internally by the hook system).
```cpp
// Pass false for Instance Methods
Method updateMethod = RuntimeHelper::GetMethodData("Player", "getHP", false);

// Pass true for Static Class Methods
Method sharedMethod = RuntimeHelper::GetMethodData("GameManager", "shared", true);
```

### 🔹 GetMethodIMP
Retrieve the actual function pointer (`IMP`) implementation of a method. Pass `true` for static class methods, or `false` for instance methods.
```cpp
// Pass false for Instance Methods
IMP updateImp = RuntimeHelper::GetMethodIMP("Game", "update", false);

// Pass true for Static Class Methods
IMP sharedImp = RuntimeHelper::GetMethodIMP("GameManager", "shared", true);
```

---

# 🧠 IVAR SYSTEM (GET / SET + OFFSET)

## 🔹 GetIvarOffset

Get memory offset of an ivar inside a class.

### Syntax
```cpp
uintptr_t off = OBJCRuntime::GetIvarOffset("ClassName", "ivarName");
```

### Examples
```cpp
uintptr_t a = OBJCRuntime::GetIvarOffset("Player", "m_hp");
uintptr_t b = OBJCRuntime::GetIvarOffset("Enemy", "m_damage");
uintptr_t c = OBJCRuntime::GetIvarOffset("Character", "m_speed");
```

---

## 🔹 Read Ivar (Value from Memory)

Read value directly from object memory using offset.

### Syntax
```cpp
OBJCRuntime::GetIvar<Type>(obj, "ClassName", "ivarName");
```

### Examples
```cpp
int a = OBJCRuntime::GetIvar<int>(obj, "Player", "m_hp");
float b = OBJCRuntime::GetIvar<float>(obj, "Player", "m_speed");
bool c = OBJCRuntime::GetIvar<bool>(obj, "Player", "m_alive");
```

---

## 🔹 Write Ivar (Modify Memory Value)

Write value directly into object memory.

### Syntax
```cpp
OBJCRuntime::SetIvar<Type>(obj, "ClassName", "ivarName", value);
```

### Examples
```cpp
OBJCRuntime::SetIvar<int>(obj, "Player", "m_hp", 999);
OBJCRuntime::SetIvar<float>(obj, "Player", "m_speed", 5.5f);
OBJCRuntime::SetIvar<bool>(obj, "Player", "m_alive", true);
```

---

# 📦 METHOD SYSTEM (CALL + INVOKE)

## 🔹 Instance Method Invocation

Dynamically invoke an instance method via string name with flexible arguments.

### Syntax
```cpp
OBJCRuntime::GetInstanceMethod<Ret>(obj, "ClassName", "selectorName", args...);
```

### Examples
```cpp
// 1. No Parameters (0 Parameters)
int hp = OBJCRuntime::GetInstanceMethod<int>(obj, "Player", "getHP");

// 2. One Parameter (1 Parameter)
bool state = OBJCRuntime::GetInstanceMethod<bool>(obj, "Player", "setActive:", true);

// 3. Multiple Parameters
float dmg = OBJCRuntime::GetInstanceMethod<float>(obj, "Player", "calculateDamageWithWeapon:target:", weaponId, targetObj);
```

---

## 🔹 Class Method Invocation

Dynamically invoke static class methods of a specific class.

### Syntax
```cpp
OBJCRuntime::GetClassMethod<Ret>("ClassName", "selectorName", args...);
```

### Examples
```cpp
// 1. No Parameters (0 Parameters)
void *mgr = OBJCRuntime::GetClassMethod<void *>("GameManager", "shared");

// 2. One Parameter (1 Parameter)
void *cfg = OBJCRuntime::GetClassMethod<void *>("ConfigManager", "loadConfigWithVersion:", 2);

// 3. Multiple Parameters
bool logged = OBJCRuntime::GetClassMethod<bool>("NetworkManager", "loginWithUser:token:type:", @"MrD", @"tkn123", 1);
```

---
# 🔍 METHOD INSPECTION & ADDRESS SYSTEM

An introspection system designed to resolve and locate active runtime memory addresses (`IMP`), static binary offsets (`RVA`), and absolute Virtual Addresses (`VA`) for both Instance and Class methods.

### 📊 Address Types Comparison

| Address Type | Key Concept | Reference Point | Main Use Case |
| :--- | :--- | :--- | :--- |
| **Real Address (`IMP`)** | Active dynamic pointer location in live iOS/macOS RAM. | Changes every launch due to ASLR. | Creating dynamic function pointers or executing native runtime hooks. |
| **RVA (Offset)** | The relative distance from the image base address. | Constant, independent of ASLR. | Locating code changes or patching binaries relative to image base. |
| **VA (Virtual Address)** | Absolute static address mapped to the binary architecture base. | Matches the static file on disk. | Locating methods by exact addresses shown in **IDA Pro** or **Ghidra**. |

---

## 🔹 Real Address (IMP)

Retrieve the dynamic, active runtime memory address (`IMP` pointer) of a method directly from live RAM. 

### 1. Instance Method Real Address
#### Syntax
```cpp
uintptr_t imp = OBJCRuntime::GetInstanceMethodRealAddress("ClassName", "selectorName");
```
#### Examples
```cpp
// 1. No Parameters (0 Parameters)
uintptr_t addrA = OBJCRuntime::GetInstanceMethodRealAddress("Player", "getHP");

// 2. One Parameter (1 Parameter)
uintptr_t addrB = OBJCRuntime::GetInstanceMethodRealAddress("Player", "setActive:");

// 3. Multiple Parameters
uintptr_t addrC = OBJCRuntime::GetInstanceMethodRealAddress("Player", "calculateDamageWithWeapon:target:");
```

### 2. Class Method Real Address
#### Syntax
```cpp
uintptr_t imp = OBJCRuntime::GetClassMethodRealAddress("ClassName", "selectorName");
```
#### Examples
```cpp
// 1. No Parameters (0 Parameters)
uintptr_t cAddrA = OBJCRuntime::GetClassMethodRealAddress("GameManager", "shared");

// 2. One Parameter (1 Parameter)
uintptr_t cAddrB = OBJCRuntime::GetClassMethodRealAddress("ConfigManager", "loadConfigWithVersion:");

// 3. Multiple Parameters
uintptr_t cAddrC = OBJCRuntime::GetClassMethodRealAddress("NetworkManager", "loginWithUser:token:type:");
```

---

## 🔹 Relative Virtual Address (RVA)

Get the static binary offset of a method relative to its loading image base address.

### 1. Instance Method RVA
#### Syntax
```cpp
uintptr_t rva = OBJCRuntime::GetInstanceMethodOffsetRVA("ClassName", "selectorName");
```
#### Examples
```cpp
// 1. No Parameters (0 Parameters)
uintptr_t rvaA = OBJCRuntime::GetInstanceMethodOffsetRVA("Player", "getHP");

// 2. One Parameter (1 Parameter)
uintptr_t rvaB = OBJCRuntime::GetInstanceMethodOffsetRVA("Player", "setActive:");

// 3. Multiple Parameters
uintptr_t rvaC = OBJCRuntime::GetInstanceMethodOffsetRVA("Player", "calculateDamageWithWeapon:target:");
```

### 2. Class Method RVA
#### Syntax
```cpp
uintptr_t rva = OBJCRuntime::GetClassMethodOffsetRVA("ClassName", "selectorName");
```
#### Examples
```cpp
// 1. No Parameters (0 Parameters)
uintptr_t rvaA = OBJCRuntime::GetClassMethodOffsetRVA("GameManager", "shared");

// 2. One Parameter (1 Parameter)
uintptr_t rvaB = OBJCRuntime::GetClassMethodOffsetRVA("ConfigManager", "loadConfigWithVersion:");

// 3. Multiple Parameters
uintptr_t rvaC = OBJCRuntime::GetClassMethodOffsetRVA("NetworkManager", "loginWithUser:token:type:");
```

---

## 🔹 Virtual Address (VA)

Get the absolute static Virtual Address of a method by subtracting the active ASLR Slide from its live implementation address. It automatically resolves to base `0x100000000` for Main Binaries and base `0x0` for dynamic libraries/frameworks (like UnityFramework).

### 1. Instance Method VA
#### Syntax
```cpp
uintptr_t va = OBJCRuntime::GetInstanceMethodVA("ClassName", "selectorName");
```
#### Examples
```cpp
// 1. No Parameters (0 Parameters)
uintptr_t vaA = OBJCRuntime::GetInstanceMethodVA("Player", "getHP");

// 2. One Parameter (1 Parameter)
uintptr_t vaB = OBJCRuntime::GetInstanceMethodVA("Player", "setActive:");

// 3. Multiple Parameters
uintptr_t vaC = OBJCRuntime::GetInstanceMethodVA("Player", "calculateDamageWithWeapon:target:");
```

### 2. Class Method VA
#### Syntax
```cpp
uintptr_t va = OBJCRuntime::GetClassMethodVA("ClassName", "selectorName");
```
#### Examples
```cpp
// 1. No Parameters (0 Parameters)
uintptr_t vaA = OBJCRuntime::GetClassMethodVA("GameManager", "shared");

// 2. One Parameter (1 Parameter)
uintptr_t vaB = OBJCRuntime::GetClassMethodVA("ConfigManager", "loadConfigWithVersion:");

// 3. Multiple Parameters
uintptr_t vaC = OBJCRuntime::GetClassMethodVA("NetworkManager", "loginWithUser:token:type:");
```
---

## 🔹 Hook Instance Method

Replace the runtime implementation (`IMP`) of an instance method with a custom hook function. This uses native method swizzling under the hood, affecting all live instances of the target class.

### ⚠️ Critical Rule for Hooks
Every Objective-C method natively carries **two hidden arguments** at the beginning of its signature:
- `id self` - The pointer to the calling object instance.
- `SEL _cmd` - The selector of the current method.

Your custom hook function signature **must** include these two hidden arguments before any explicit parameters.

### Syntax
```cpp
bool success = OBJCRuntime::HookInstanceMethod("ClassName", "selectorName", (void*)&myHookFunc, (void**)&origFuncPointer);
```

### Examples
```objc
// 1. No Parameters (0 Parameters)
void (*orig_getHP)(id self, SEL _cmd);
void my_getHP(id self, SEL _cmd) {
    orig_getHP(self, _cmd);
}

// 2. One Parameter (1 Parameter)
void (*orig_setActive)(id self, SEL _cmd, bool active);
void my_setActive(id self, SEL _cmd, bool active) {
    orig_setActive(self, _cmd, active);
}

// 3. Multiple Parameters
void (*orig_calcDamage)(id self, SEL _cmd, void* weapon, void* target);
void my_calcDamage(id self, SEL _cmd, void* weapon, void* target) {
    orig_calcDamage(self, _cmd, weapon, target);
}
```

---

## 🔹 Hook Class Method

Replace the runtime implementation (`IMP`) of a static class method by routing structural changes through the internal Meta-class table.

### ⚠️ Critical Rule for Hooks
Just like instance methods, class method hooks **must** include the two hidden arguments, but the first argument is the class structure rather than an object instance:
- `Class self` - The pointer to the structural Class object.
- `SEL _cmd` - The selector of the current method.

### Syntax
```cpp
bool success = OBJCRuntime::HookClassMethod("ClassName", "selectorName", (void*)&myHookFunc, (void**)&origFuncPointer);
```

### Examples
```objc
// 1. No Parameters (0 Parameters)
void* (*orig_shared)(Class self, SEL _cmd);
void* my_shared(Class self, SEL _cmd) {
    return orig_shared(self, _cmd);
}

// 2. One Parameter (1 Parameter)
void (*orig_loadConfig)(Class self, SEL _cmd, const char* version);
void my_loadConfig(Class self, SEL _cmd, const char* version) {
    orig_loadConfig(self, _cmd, version);
}

// 3. Multiple Parameters
bool (*orig_login)(Class self, SEL _cmd, const char* user, const char* token, int type);
bool my_login(Class self, SEL _cmd, const char* user, const char* token, int type) {
    return orig_login(self, _cmd, user, token, type);
}
```

---

# 🔥 USE CASES

- iOS game modding
- Reverse engineering applications
- Runtime manipulation & tool development
- Debugging internal app state
- Memory modification utilities

---

# ⚠️ NOTES

- Offsets depend on binary version
- Raw memory access must be used carefully to avoid Segmentation Faults
- Ensure the target objects are alive before calling `GetIvar`/`SetIvar` or invoking methods

---

# 📌 AUTHOR

- [DS Gaming - Mr D](https://github.com/dsgaming-mrd)

This project is my Wine development repository in which I prepare my Vulkan patches.
It is meant for showing the development work without blasting wine-devel with huge
patchsets.

You can use this repo to play around with Vulkan on Wine, but this repo is not meant
to be stable. It will be rebased frequently, so be warned!

In order to use wine-vulkan:
1. Run autoconf at the root of wine-vulkan to create configure script.
2. Compile Wine as usual.
3. Then download and install the Windows Vulkan SDK for the Vulkan loader: https://www.lunarg.com/vulkan-sdk/
4. Create a json file "c:\\windows\\winevulkan.json" containing:
```
{
    "file_format_version": "1.0.0",
    "ICD": {
        "library_path": "c:\\windows\\system32\\winevulkan.dll",
        "api_version": "1.0.51"
    }
}
```
5. Add registry key(s):
```
[HKEY_LOCAL_MACHINE\SOFTWARE\Khronos\Vulkan\Drivers\]
"C:\Windows\winevulkan.json"=dword:00000000
```

If on 64-bit also add a line to load the json file for 32-bit:
```
[HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432NodeKhronos\Vulkan\Drivers\]
"C:\Windows\winevulkan.json"=dword:00000000
```

At the moment the patchset provides Vulkan 1.0.51 core with minimal extensions for graphics
rendering. For licensing reasons vk.xml (to be resolved soon), it is not supporting newer
versions. The code is enough to run some basic Vulkan test applications on 64-bit
Wine including 'The Talos Principle', 'Doom', 'Wolfenstein II', 'cube.exe', 'vulkaninfo.exe' and 'VkQuake'.

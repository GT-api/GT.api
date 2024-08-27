<div align="center">

[![](https://github.com/GT-api/GT.api/actions/workflows/msys2.yml/badge.svg)](https://github.com/GT-api/GT.api/actions/workflows/msys2.yml)
[![](https://app.codacy.com/project/badge/Grade/96ff25f75aa24cd38d694a09140839c9)](https://app.codacy.com/gh/LeeEndl/GT.api/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![](https://dcbadge.limes.pink/api/server/AQxDjbDt?style=flat)](https://discord.gg/AQxDjbDt)
</div>

***
### ![checklist](https://github.com/microsoft/vscode-icons/blob/main/icons/dark/checklist.svg) Requirements

- [VSC](https://code.visualstudio.com/)
  - (optional) install C++ extention in VSC
- [MSYS2](https://www.msys2.org/) (Follow the intructions)
  - after the installation of **MSYS2**:
    - Press <img src="https://www.servis-repas.cz/user/categories/orig/windows-11-icon.png" width="20" height="20"></img> and type 'cmd' and input: `C:\msys64\ucrt64.exe`
    - input inside the popup console: `pacman -Suy && pacman -S mingw-w64-ucrt-x86_64-curl mingw-w64-ucrt-x86_64-jsoncpp`
### ![build](https://github.com/microsoft/vscode-icons/blob/main/icons/dark/build.svg) Building 
- Press <img src="https://www.servis-repas.cz/user/categories/orig/windows-11-icon.png" width="20" height="20"></img> and type 'cmd' and input: `setx PATH "%PATH%;C:\msys64\ucrt64\bin"`
- Open VSC in `GT.api-main` folder, then press `Ctrl + Shift + B` to start compiling.

<div align="center">

[![](https://github.com/GT-api/GT.api/actions/workflows/make.yml/badge.svg?branch=main&event=push)](https://github.com/GT-api/GT.api/actions/workflows/make.yml)
[![](https://app.codacy.com/project/badge/Grade/fa8603d6ec2b4485b8e24817ef23ca21)](https://app.codacy.com/gh/gurotopia/gurotopia/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![](https://dcbadge.limes.pink/api/server/zzWHgzaF7J?style=flat)](https://discord.gg/zzWHgzaF7J)

</div>

***
### ![checklist](https://github.com/microsoft/vscode-icons/blob/main/icons/dark/checklist.svg) Requirements

- [VSC](https://code.visualstudio.com/)
  - (optional) install C++ extension in VSC
- [MSYS2](https://www.msys2.org/) (Follow the intructions)
  - after installing **MSYS2**:
    - Press <img src="https://www.servis-repas.cz/user/categories/orig/windows-11-icon.png" width="20" height="20"></img> and type 'cmd' and input: `C:\msys64\ucrt64.exe`
    - input inside the popup console: `pacman -Syu && pacman -S --needed mingw-w64-ucrt-x86_64-gcc make`
### ![build](https://github.com/microsoft/vscode-icons/blob/main/icons/dark/build.svg) Building 
- Press <img src="https://www.servis-repas.cz/user/categories/orig/windows-11-icon.png" width="20" height="20"></img> and type 'cmd' and input: `setx PATH "%PATH%;C:\msys64\ucrt64\bin;C:\msys64\usr\bin"`
- Open VSC in `gurotopia-main` folder, then press `Ctrl + Shift + B` to start compiling.

***
> [!NOTE]
> Make sure 'hosts' file via `C:/Windows/System32/drivers/etc/` is set to:
> ```
> 127.0.0.1 www.growtopia1.com
> 127.0.0.1 www.growtopia2.com
> ```

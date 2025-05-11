<div align="center">

**グロートピア** *(Gurotopia)* : Lightweight & Maintained GTPS written in C/C++

[![](https://github.com/GT-api/GT.api/actions/workflows/make.yml/badge.svg?branch=main&event=push)](https://github.com/GT-api/GT.api/actions/workflows/make.yml)
[![](https://app.codacy.com/project/badge/Grade/fa8603d6ec2b4485b8e24817ef23ca21)](https://app.codacy.com/gh/gurotopia/gurotopia/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![](https://dcbadge.limes.pink/api/server/zzWHgzaF7J?style=flat)](https://discord.gg/zzWHgzaF7J)

</div>

***
1. __Download:__
   - **Visual Studio Code**:  **https://code.visualstudio.com/**
   - **MSYS2**:  **https://www.msys2.org/**
   - **C/C++ extension for VSC**: https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools

2. __Installing MSYS packages__:
   - Locate MSYS folder (`C:\msys64`)
   - Open **ucrt64.exe** and paste **`pacman -Syu`** into the terminal
   - Now, re-open **ucrt64.exe** and paste **`pacman -S --needed mingw-w64-ucrt-x86_64-gcc make`** into the terminal

3. __Compile__:
   - After downloading *Gurotopia*, open it in Visual Studio Code, typically compiling is taken place in VSC terminal manually, however some projects including mine has a build shortcut (**Ctrl + Shift + B**), and let it compile...

4. __Debugging or Running__:
   - after compile press **F5** key- this will run and debug, however debugging is unnecessary if you wish to host a GTPS, please press **Ctrl + F5** instead.
   - *(optional)* how to stop the server? press **Shift + F5**, or simply close VSC.
   - *(optional)* how to restart the server? press **Ctrl + Shift + F5**.
***

> [!NOTE]
> Make sure 'hosts' file via `C:/Windows/System32/drivers/etc/` is set to:
> ```
> 127.0.0.1 www.growtopia1.com
> 127.0.0.1 www.growtopia2.com
> ```

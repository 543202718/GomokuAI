# GomokuAI
+ AI.cpp是五子棋AI的源代码
+ AI.exe是五子棋AI的可执行文件，通过在命令行输入AI执行。在程序执行时，AI落子的坐标会通过直接输出在命令行中，用户可以通过输入自己落子的坐标与之交互。在程序结束时，对局会被写入同一目录下的output.txt文件中
+ 通过下面的命令编译AI.cpp
    > g++ -o AI.exe AI.cpp -std=c++11
+ 通过命令行下棋并不方便，GomokuGUI.jar是一个图形化的五子棋界面，它会调用AI.exe并与之交互。用户在界面上落子的位置会被传递给AI.exe，同时AI.exe落子的位置会被显示在界面上。通过在命令行输入下面的命令运行该程序：
    > javaw -jar GomokuGUI.jar 
+ image目录中是GUI依赖的图片，gomokugui目录中是它的源代码
+ AI落子大概需要1-2s，最多不会超过5s

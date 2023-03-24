# Shell-Emulator
C Program to emulate the functions of a command line interface

Mostly works fine, with some hiccups.

The command that gave me the most trouble was the ‘history’. Though it works fine if you implement 10 commands and type ‘history’, the implementation here has trouble with any amount more or less than that. If you type in 4 commands and then ‘history’, the shell woill output the commands in a haphazard order, with one command following the ‘2.’, for instance, and another following the ‘9.’ All of the other spaces would be blank. It was hard to figure out a way to get the shell to output only 4 commands without affecting other parts of the program. If more than 10 commands are entered, the shell decides to loop back around, with the latest command replacing the very first command entered. Again, while I had some ideas for fixing this, I had trouble doing so without changing other parts of the program. 

Another command that gave me trouble was the ‘sort’ command, mainly because it was tedious to implement. I had to do a bit of studying on redirection to properly implement it. Multiple implementations were iffy on spaces. With one version of the program, both ‘sort<file1.txt’ and ‘sort < file1.txt’ worked fine, while in another, only the latter is correct. This seemed to be how most shells performed, so I kept the latter implementation.

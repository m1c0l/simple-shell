# Simple shell

This is called the simple shell, but it's definitely not simple. It has bash-like functionality, but is much more verbose and has more features, namely the ability to open files with many of `open()`'s flags, and support for direct-edge dependencies when running shell commands. Shell commands are run with 3 file descriptors: a standard input, standard output, and standard error, and when combined with pipes, you can create more complicated schedules of commands than with a shell like bash, which only provides a linear-like schedule of commands. 

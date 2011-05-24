
This project contains code that :-

- launches a child process
- redirects the child process's stdout and stderr to pipes
- waits for the child process to exit
- makes the data written to stdout and stderr available to the caller

The Visual Studio 2008 solution contains 2 Visual C++ projects :-

- ProcessRedirection which is a static library
- TestHarness which is a console application

I have not used Windows pipes before so I was experimenting somewhat.
Therefore, the static library actually contains 2 different implementations
of this functionality :-

1) Using anonymous pipes - this version launches separate threads to
   read the stdout and stderr pipes using blocking operations.

2) Using named pipes and asynchronous I/O - this version does everything
   on the current thread. It uses an alertable wait to wait for the
   child process to exit which allows our ReadFileEx completion routines
   to execute.

TODO :-

- sort out areas of code duplication e.g. reading of stdout/stderr - the code
  is very similar so I should be able to make it more elegant

- since stuff written to stdout and stderr is usually text, it might
  be convenient to make it available as a string array where each element
  is a line i.e. split the output on line breaks.


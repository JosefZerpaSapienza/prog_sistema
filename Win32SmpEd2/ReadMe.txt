March 15, 2002

Update: Fix cat, atou projects so that the run, lib, and include
directory pathnames are relative to the project. I need to complete
this for all projects and will do so in the near future.
---------------------------

DESCRIPTION

This disc contains the source code for all the sample programs as well as the 
include files, utility functions, and projects. A number of programs illustrate 
additional features and solve specific exercises, although the disc does not 
include solutions for all exercises or show every alternative implementation.
	All programs have been tested running under Windows 2000 and NT on Intel 
Pentium II and III systems. Where appropriate, they have also been tested under 
Windows 95 and Windows 98.
	The programs have been built and run with and without UNICODE defined 
under 2000/NT. Under Windows 9x, only the non-Unicode versions will operate.
	Nearly all programs compile without warning messages under Microsoft 
Visual C++ version 6.0 using warning level 3. There are a few exceptions, such 
as warnings about "no return from main program" when ExitProcess is used.
	The Visual C++ generic C library functions are used extensively, as are 
compiler-specific keywords such as __try, __except, and __leave. The 
multithreaded C run-time library, _beginthreadex, and _endthreadex are essential 
starting with Chapter 8.
	The projects (in release, not debug, form) and make files are included. 
The projects are all very simple, with minimal dependencies, and can also be 
created quickly with the desired configuration and as either debug or release 
versions.
	Build all programs, with the exception of static or dynamic libraries, as 
console applications; projects are included.

Disc Organization
The primary directory is named Win32Smp ("Win32 Sample Programs"), and this 
directory can be copied directly to the hard disc. There is a subdirectory for 
each chapter. All include files are in the Include directory, and the Utility 
directory contains the commonly used functions such as ReportError and 
PrintStrings. Complete projects are in the Projects directory. Executable and 
DLLs for all projects are in the Run directory. The TimeTest directory contains 
files required to run the performance tests described in Appendix C.

The Utility Directory
The Utility directory contains six source files for utility functions required 
by the sample programs.
1.	ReprtErr.c contains the functions ReportError (Program 2-2) and 
ReportException (see Chapter 4-1). Every program executed as a process by other 
sample programs requires this file, except for the grep and wc programs and 
those in Chapter 1.
2.	PrintMsg.c contains PrintStrings, PrintMsg, and ConsolePrompt (Program 2-
1). ReprtErr.c calls these functions, so this source file is also required in 
nearly every project.
3.	Options.c contains the function that processes command line options and is 
used frequently starting in Chapter 2. Include this source file in the project 
for any program that has command line options. The listing is Program A-7.
4.	Wstrings.c contains the source code for the wmemchr function used by 
Options.c. Include this file as necessary. You may find it convenient to add 
other generic string processing functions.
5.	SkipArgs.c processes a command line by skipping a single argument field 
with each call. It is listed in Program A-8.
6.	GetArgs.c converts a character string into the argc, argv [] form. This 
program is useful when you're converting a command into a DLL or to process 
results from the GetCommandLine function introduced in Chapter 7. The listing is 
Program A-9.

The functions can be compiled and linked with the calling program. You will find 
it easiest, however, to build them as a library, either static or dynamic. If 
they are to be part of a DLL, you will need to define the _UTILITY_EXPORTS 
variable in support.h to export their definitions. The utility project builds a 
DLL from these source files, while utilityStatic is the project to create a 
static library.

The Include Directory
The include files are as follows:
1.	EvryThng.h, which, as the name suggests, brings in nearly everything 
required for normal programs, whether single-threaded or multithreaded. In 
particular, it includes the files Envirmnt.h and Support.h. The listing is 
Program A-1.
2.	Exclude.h defines a number of preprocessor variables that exclude 
definitions not required by any of the programs in the book. This arrangement 
speeds compilation and reduces the size of the precompiled header files.
3.	Envirmnt.h defines the UNICODE and _UNICODE preprocessor variables 
consistently as well as the language and sublanguage used by ReportError. 
Program A-2 lists this file.
4.	Support.h defines many of the common functions, such as ReportError, as 
well as a variety of frequently used symbolic constants. Chapter A-3 shows this 
file.
5.	ClntSrvr.h is used starting in Chapter 11. It defines the request and 
response message structures, client and server named pipes and mailslots, time-
out values, and so on. See Program A-4.
6.	JobMgt.h is used in the job management programs at the end of Chapter 7. 
See Program A-5.

Programs by Chapter
Each chapter directory contains all the programs in that chapter (except for the 
programs in the Utility directory) as well as miscellaneous additional programs. 
The programs are listed here, with brief descriptions for the additional 
programs. You will also find a number of programs with an "x" suffix; these 
programs contain deliberate defects which illustrate common programming errors.

Chapter 1
	cpC.c is Program 1-1.
	cpW.c is Program 1-2; cpwFA.c shows the code modified for better 
performance. See the results in Appendix C.
	cpCF.c is Program 1-3.
	Other programs include a UNIX version (cpU.c) and one (cpUC.c) built to 
use the very limited UNIX compatibility library provided with Visual C++.

Chapter 2
	cat.c is Program 2-3.
	atou.c is Program 2-4.
	Asc2Un.c is Program 2-5; Asc2UnFA.c and Asc2UnNB.c are performance-
enhanced versions. All three files implement the Asc2Un function called by 
Program 2-5.
	pwd.c is Program 2-6; pwda.c is modified to allocate the required memory 
for the pathname.
	cd.c is an implementation of the UNIX directory change command; it is not 
an example in Chapter 2.

Chapter 3
	tail.c is Program 3-1.
	ls.c is Program 3-2. rm.c is a similar program to remove files.
	touch.c is Program 3-3.
	getn.c is an additional program that reads a specified fixed-size record, 
illustrating file access and computing file positions.
	lsReg.c is Program 3-4.
	FileSize.c is an exercise solution that determines whether file space is 
allocated sparsely or not.
	TestLock.c exercises file locking.

Chapter 4
	Program 4-1 is part of ReprtErr.c in the Utility directory.
	toupper.c is Program 4-2.
	Excption.c is Program 4-3 and contains a filter function, Program 4-4.
	Ctrlc.c is Program 4-5.

Chapter 5
	chmod.c is Program 5-1.
	lsFP.c is Program 5-2.
	InitUnFp.c is the code forProgram 5-3, Program 5-4, and Program 5-5. 
Program 5-1 and Program 5-2 require these functions. The source module also 
contains code showing how to obtain the name of an owning group, which is a 
Chapter 7 exercise.
	TestFp.c is an additional test program that was useful during testing.

Chapter 6
	sortBT.c is Program 6-1 and Program 6-2; sortBTSR.c omits the no-
serialization option on memory management calls to determine whether there is 
any performance impact in a simple application. The reader can verify that there 
is very little effect.
	Asc2UnMM.c is the function for Program 6-3. 
	sortFL.c is Program 6-4, and sortHP.c is a similar program except that it 
reads the file into an allocated memory buffer rather than using mapped memory.
	sortMM.c is Program 6-5 and Program 6-6.
	atouEL.c is Program 6-7, and Asc2UnDll.c and Asc2UnmmDLL.c are the source 
files for the required DLLs.
	HeapNoSr.c is a test program that measures the effect of memory allocation 
with and without the HEAP_NO_SERIALIZE flag. This program can be used with two 
of the exercises.
	clear.c is a simple program that allocates and initializes virtual memory 
in large units, continuing until failure. This program is used between timing 
tests to ensure that data is not cached into memory, distorting the 
measurements.

Chapter 7
	grepMP.c is Program 7-1. grep.c is the source for a C library-based 
pattern search program to be invoked as a process by grepMP.c. 
	timep.c is Program 7-2.
	JobShell.c is Program 7-3, and JobMgt.c provides the support functions of 
Program 7-4, Program 7-5, and Program 7-6.
	catHA.c and grepMPha.c are modified versions of other programs designed to 
show how to pass a handle on the command line, solving an exercise problem.

Chapter 8
	grepMT.c is Program 8-1.
	sortMT.c is Program 8-2.
	wcMT.c solves an exercise.
Building the multithreaded applications requires the LIBCMT.LIB library when 
using Visual C++, and it is necessary to suppress the default library. This can 
be accomplished with the following steps:
1.	From the main menu, select Build...Settings...Link.
2.	Select the Ignore All Default Libraries checkbox.
3.	Enter LIBCMT.LIB in the Object/Library Modules: window.
An even better method, used with all the supplied projects, is the following, 
with Visual C++:
1.	From the main menu, select Build...Settings...C/C++.
2.	Under Category, select CodeGeneration.
3.	Select the appropriate multithreaded library.
This technique will define _MT on the command line generated to invoke the 
compiler.

Chapter 9
	simplePC.c is Program 9-1.
	eventPC.c is Program 9-2.
	statsMX.c is Program 9-3. Variations are statsNS.c, statsCS.c, and 
statsIN.c.
	TimeMutualExclusion.c is used for timing studies suggested in the 
exercises.

Chapter 10
	Program 10-1 contains part of SynchObj.h; Program 10-3 contains the rest.
	ThbObject.c is Program 10-2. testTHB.c is the associated test program.
	QueueObj.c is Program 10-4, and variations include QueueObjCS.c (uses a 
CRITICAL_SECTION), QueueObjSOAW.c (uses SignalObjectAndWait), and signal model 
versions.
	ThreeStage.c is Program 10-5, and its project requires Messages.c and 
QueueObj.c.
	MultiSem.c, along with the test program, TestMultiSem.c, is the solution 
to an exercise.

Chapter 11
	pipe.c is Program 11-1. wc.c is used as a convenient command to 
demonstrate its operation.
	clientNP.c is Program 11-2.
	serverNP.c is Program 11-3.
	SrvrBcst.c is Program 11-4.
	LocSrver.c is Program 11-5.

Chapter 12
	clientSK.c is Program 12-1.
	serverSK.c is Program 12-2.
	command.c is Program 12-3.
	SendReceiveSKST.c is Program 12-4, and serverSKST.c and clientSKST.c are 
slight modifications of serverSK.c and clientSK.c for streaming I/O. 
SendReceiveSKST.c should be built as a DLL, and the DLL should be implicitly 
linked with the client and server projects.
	SendReceiveSKHA.c is Program 12-5, and serverSKHA.c is the corresponding 
server to use the DLL. clientSKST will work with this server.

Chapter 13
	SimpleService.c is Program 13-2.
	ServiceShell.c is Program 13-3.
	serviceSK.c is serverSK.c (Program 12-2) converted to a service.

Chapter 14
	atouOV.c is Program 14-1.
	atouEX.c performs the same task with extended I/O and is Program 14-2.
	atouMT.c performs the same task with multiple threads rather than with 
Win32 asynchronous I/O.
	atouMTCP.c uses I/O completion ports.
	TimeBeep.c is Program 14-3.
	serverCP, Program 14-4, is a version of serverMT that uses I/O completion 
ports and overlapped I/O.

Chapter 15
	SysCommand.IDL is Program 15-1.
	SysCommandClient.c and SysCommandServer.c represent Program 15-2 and 
Program 15-3, respectively.
	SysCommand_c.c and Syscommand_s.c are the MIDL generated client and server 
stubs, while SysCommand.h is the generated header file.


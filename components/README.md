# Libraries
Most logic of all boards should be housed in this directory.
These should alls be **agnostic** to everything besides themselves (and maybe some other libraries: NOT FIRMWARE!!)
This is because unit testing cannot be done alongside firmware specific libraries (uart communication, pin i/p, etc)

## Common libraries

These libraries are meant to be used by multiple boards at the same time.
You can create one but try to keep it for only system critical part.
If a library is for only *one* board please place it under that specific board's library directory.

## Board Specific libraries
Where most of the logic for each board should lead.
The file structure should be similar to:

lib/{board}
|- {library}/
    |- {library.h}
    |- {library.c}

for testing each library should be tested like this:
test/{board}
|- test_{library}
    |- {test_library_behaviour1}.c
    |- {test_library_behaviour2}.c 


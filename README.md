# ocwiper - (o)pen ( c ) file wiper
---
Like SDelete, but open-source, versatile and hackable.

#### Why use ocwiper?
---
1. Open-source
2. Cross platform
3. Easily modifiable
4. Usable with software built for SDelete (such as [my GUI](https://github.com/compl3x/SDelete2-FileGone))

#### Building
---
A script, build.sh, is included in the repo for quick build. it's basically just:

    gcc -o ocwiper wipe.c mtwist.c

#### To-Do
---
* ~~Directory support~~ DONE! 
* ~~Recursive Directory support~~ DONE!
* ~~-f command~~ DONE! Use -a
* Filename scrambling


#### Program Flags
---
##### -k
Keeps the file after wiping
##### -a
Removes the read-only attribute from a file (and wipes it)
##### -q
Supress all output/errors
##### -p passes
Overwrite the file a custom number of times (the default is 5)
##### -r
Recurse through directories

---
(c) Michael Cowell, 2015

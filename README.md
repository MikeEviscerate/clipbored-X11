# Clipbored-X11
A header including two functions, one for copying to and one for pasting from clipboard in X11.

I am new to making Markdown and such, as well as me being inexperienced at using GitHub. So, this is going to be weird. 

Note: I did use code from [here](https://github.com/exebook/x11clipboard). It wasn't licensed but I did get permission from **exebook** to use it.

# What does it do?
It is header file neatly wrapping and hiding all the BS of copying and pasting in C for X11 into two functions.
`XPaste()` for pasting from clipboard, returns a string. `XCopy()` takes a string as input and copies it to clipboard. 

# How to use it? (Important)
Just include the header in your source. 

Well, you will need to initialise things. I have wrapped it up into the function macro `clipBoredInit(enableCopying)`. Preferably, use it in your 
first line of `main()`.

`enableCopying` is just a boolean input, use `true` or `1` for enabling copying. Or else, use `false` or `0`. Afterwards, you 
can use `XCopy()` and `XPaste()` anywhere. If you don't call it before using those functions, it will cause a segmentation fault. 

I suggest you look into how `clipBoredInit()` is defined so that you can alter it to be better in your case. 

I didn't try including it in C++ source files at the time of writing this. I don't know if it will work with C++ stuff. 

# Why create it?
No one really made it before. And there are bunch of newbies who can take advantage of it.

# Tested platforms
<table>
<thead>
<tr>
<th>OS</th>
<th>Compiler</th>
<th>Works?</th>
</tr>
</thead>
<tbody>
<tr>

<td>Arch Linux</td>
<td>gcc</td>
<td>✅</td>
</tr>
<tr>
<td>Arch Linux</td>
<td>clang</td>
<td>✅</td>
</tr>
<tr>
<td>Solaris 11.4</td>
<td>gcc</td>
<td>✅</td>
</tr>
<tr>
<td>Solaris 11.4</td>
<td>clang</td>
<td>✅</td>
</tr>
<tr>
</tr>
<tr>
<td>FreeBSD 13.1</td>
<td>gcc</td>
<td>✅</td>
</tr>
<tr>
<td>FreeBSD 13.1</td>
<td>clang</td>
<td>✅</td>
</tr>
</tbody>
</table>

# Bad code and practices? 
Suggest good code and/or fork. Some stuff was intended. Some wasn't.

**Definitions in a header file?**  
It was really convenient. 

**Why am I forking and using (grand) child for copying instead of using threads?**   
So that you can paste the copied data long after the program dies, or at least until you copy something else.

It is not thread-safe. But what do you expect from a noob? Don't expect much from it. Rest assured, I will update the code time to time, but it is not a guarantee.

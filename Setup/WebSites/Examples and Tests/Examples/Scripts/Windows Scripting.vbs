Option Explicit

Dim gStdIn, gStdOut
Set gStdIn = WScript.StdIn
Set gStdOut = WScript.StdOut

Call Main

Sub Echo(Text)
    gStdOut.WriteLine Text
End Sub

Sub Main
    Echo "Content-Type: text/html" & VBCRLF & VBCRLF
    
    Echo "<!--#include virtual=""/@PageTop.Html""-->"
    Echo "<h2>VBS Test</h2>"
    Echo "<B>This is a test VBS file!</B>" & VBCRLF
    Echo "<!--#include virtual=""/@PageBottom.Html""-->"
End Sub

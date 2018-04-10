print "Content-type: Text/Html\n\n"

print "<!--#include virtual=\"/\@PageTop.Html\"-->"
print "<h2>Python Test</h2>"
print "<b>Requestion details:</b><br />"
print "<blockquote>"

import time

iVal = 100
print "<b>Integer:</b> %d <br />" % iVal

fVal = 3.141592653589
print "<b>Float:</b> %.4f <br />" % fVal

Time = time.localtime()
iHour = Time.tm_hour
iMinute = Time.tm_min
iSecond = Time.tm_sec
i12Hour = iHour

if iHour > 12: i12Hour -= 12

print "<b>Is currently</b> %d:%d.%d <b>, I am currently:</b> " % (i12Hour, iMinute, iSecond)
if iHour < 8: print 'Sleeping'
elif iHour < 9: print 'Commuting'
elif iHour < 17: print 'Working'
elif iHour < 18: print 'Commuting'
elif iHour < 20: print 'Eating'
elif iHour < 22: print 'Resting'
else: print 'Sleeping'

print "<!--#include virtual=\"/\@PageBottom.Html\"-->"

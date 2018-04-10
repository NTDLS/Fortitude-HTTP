#!C:/strawberry/perl/bin/perl.exe
print "Content-type: text/html\n\n";

print<<EOF;
  <!--#include virtual="/\@PageTop.Html"-->
  
  <h2>PERL Test</h2>

  <b>Requestion details:</b><br />
  <blockquote>
EOF

foreach $var (sort(keys(%ENV)))
{
    $val = $ENV{$var};
    $val =~ s|\n|\\n|g;
    $val =~ s|"|\\"|g;
    print "<b>${var}</b> = \"${val}\" <BR>";
}

print<<EOF;
  </blockquote><br /><br />
  <!--#include virtual="/\@PageBottom.Html"-->
EOF

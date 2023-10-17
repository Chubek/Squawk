END   {  print "end action";  }
BEGIN {  print $0, $1, $2;    }
BEGIN {  print $2, $3, $1;    }
BEGIN {  system("echo foo");  }

BEGIN { getline <  "test_data/mail-list";  }
BEGIN { print $1; close("test_data/mail-list"); }


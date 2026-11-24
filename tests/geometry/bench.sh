prefs columns 70
resizebench() { for j in 1 2 3 4 5; do prefs columns 50; prefs columns 70; done; }
fixture() { clear; for j in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do echo abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ; done; }
fixture
time resizebench
fixture
time resizebench
fixture
time resizebench
prefs
status

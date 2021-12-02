cd ..
if exist y_tab.c del y_tab.c 
if exist lex_yy.c del lex_yy.c
win_flex_bison\win_bison.exe parser.y -d -y
win_flex_bison\win_flex.exe --never-interactive lexer.l y.tab.c
rem IDE doesnt like multiple dots in filenames
ren y.tab.c y_tab.c
ren lex.yy.c lex_yy.c
cd wpj


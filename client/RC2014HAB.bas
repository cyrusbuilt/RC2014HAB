10 REM Global variables
20 DP=0 : REM Default address for the Digital I/O card. This needs to match address setting on card.
30 IV=0 : REM Input value from I/O
40 V$="1.0" : REM Application version
50 STAT_UNKNOWN%=0 : REM Status is unknown
60 STAT_GG_OPEN%=1 : REM Garage door is open
70 STAT_GG_CLOSED%=2 : REM Garage door is closed
80 STAT_GG_AJAR%=3 : REM Garage is ajar
90 CMD_GG_ACTIVATE%=1 : REM Activate garage door command
100 CMD_DISABLE%=255 : REM Disable RC2014HAB
110 CMD%=0 : REM Initialize command parameter
120 GOTO 930

200 REM Show the status screen
210 ST=INP(DP)
220 A$="UNKNOWN"
230 B$="      "
240 IF ST=STAT_GG_OPEN% THEN A$="OPEN"
250 IF ST=STAT_GG_AJAR% THEN A$="AJAR"
260 IF ST=STAT_GG_CLOSED% THEN A$="CLOSED"
270 IF LEN(A$)>4 THEN B$="    "
280 IF LEN(A$)=7 THEN B$="   "
290 PRINT CHR$(12)
300 PRINT "********** STATUS **********"
310 PRINT "*                          *"
320 PRINT "* GARAGE STATUS: " + A$ + B$ + "*"
330 PRINT "*                          *"
340 PRINT "* 1) REFRESH               *"
350 PRINT "* 2) GO BACK               *"
360 PRINT "*                          *"
370 PRINT "****************************"
380 PRINT ""
390 INPUT "ENTER SELECTION (1/2)", SS$
400 IF SS$="1" THEN GOTO 210
410 IF SS$="2" THEN GOTO 930 ELSE GOTO 270

500 REM Wait for input from the Digital I/O card
510 WAIT DP,IV
520 RETURN

550 REM Pause for T milliseconds
560 FOR I=0 TO T STEP 1
570 NEXT
580 RETURN

600 REM Garage door control screen
610 ST=INP(DP)
620 A$="UNKNOWN"
630 IF ST=STAT_GG_OPEN% THEN A$="CLOSE"
640 IF ST=STAT_GG_CLOSED% THEN A$="OPEN"
650 B$="                  "
660 IF LEN(A$)>4 THEN B$=B$ + " "
670 PRINT CHR$(12)
680 PRINT "********** GARAGE CONTROL **********"
690 PRINT "*                                  *"
700 PRINT "* 1) ACTIVATE                      *"
710 PRINT "* 2) GO BACK                       *"
720 PRINT "*                                  *"
730 PRINT "* STATUS: " + A$ + B$ + "*"
740 PRINT "*                                  *"
750 PRINT "************************************"
760 PRINT ""
770 INPUT "ENTER SELECTION (1/2)", SEL$
780 IF SEL$="1" THEN GOTO 800
790 IF SEL$="2" THEN GOTO 930 ELSE GOTO 600
800 CMD%=CMD_GG_ACTIVATE% : GOSUB 4000
810 GOTO 600

930 REM Display main control/status menu
940 PRINT CHR$(12)
950 PRINT "********** RC2014HAB " V$ " *********"
960 PRINT "*                                    *"
970 PRINT "*      By: Cyrus Brunner 2020        *"
980 PRINT "*                                    *"
990 PRINT "* 1) SHOW STATUS                     *"
1000 PRINT "* 2) GARAGE CONTROL                  *"
1010 PRINT "* 3) EXIT                            *"
1020 PRINT "*                                    *"
1030 PRINT "**************************************"
1040 PRINT ""
1050 INPUT "ENTER SELECTION (1/2/3)", S$
1060 IF S$="1" THEN GOTO 200
1070 IF S$="2" THEN GOTO 600
1080 IF S$="3" THEN END ELSE GOTO 930

2000 REM Wait for door open
2010 ST=INP(DP)
2020 IF ST<>2 THEN GOTO 2010
2030 IF ST<>3 THEN GOTO 2010
2040 RETURN

3000 REM Wait for door close
3010 ST=INP(DP)
3020 IF ST<>2 THEN GOTO 3010
3030 IF ST<>1 THEN GOTO 3010
3040 RETURN

4000 REM Send command CMD%, then clears CMD%.
4010 IF CMD%>0 THEN OUT DP,CMD%
4020 CMD%=0
4030 RETURN

5000 REM Disable the system
5010 CMD%=CMD_DISABLE% : GOSUB 4000
5020 RETURN
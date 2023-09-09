set file1=TESTSWE_UT_LP_4_original
set file2=STN01_ACCA
set file3=STN01_Sogelink
set file4="Quadri Railway_STN01"

CheckAlignmentCMD.exe %file1%.ifc test-%file1%.json 0.0001 0.0001
CheckAlignmentCMD.exe %file2%.ifc test-%file2%.json 0.0001 0.0001
CheckAlignmentCMD.exe %file3%.ifc test-%file3%.json 0.0001 0.0001
CheckAlignmentCMD.exe %file4%.ifc test-%file4%.json 0.0001 0.0001
pause:





















mkdir .\..\..\..\driver
mkdir .\..\..\..\driver\2000
mkdir .\..\..\..\driver\xp

copy customerRel\art.exe .\..\..\..\
copy ..\devlib\customerRel\devlib.dll .\..\..\..\
copy ar5001a_cb.eep .\..\..\..\
copy ar5001ap_ap.eep .\..\..\..\
copy ar5001a_mb.eep .\..\..\..\
copy ar5001x_cb.eep .\..\..\..\
copy ar5001x_mb.eep .\..\..\..\

copy uninst_old_drv.bat .\..\..\..\
copy inst_new_drv_2k.bat .\..\..\..\
copy inst_new_drv_xp.bat .\..\..\..\
copy uninst_new_drv_2k.bat .\..\..\..\
copy artsetup.txt .\..\..\..\
copy atheros-eep.txt .\..\..\..\
copy macid.txt .\..\..\..\
copy calsetup.txt .\..\..\..\
copy calTrgtPwr_ar5001ap_ap.txt .\..\..\..\
copy calTrgtPwr_ar5001a_cb.txt .\..\..\..\
copy calTrgtPwr_ar5001x_cb.txt .\..\..\..\
copy calTrgtPwr_ar5001x_mb.txt .\..\..\..\
copy calTrgtPwr_ar5001a_mb.txt .\..\..\..\

copy driver\anwi.inf .\..\..\..\
copy driver\2000\anwiwdm.sys .\..\..\..\driver\2000
copy driver\xp\anwiwdm.sys .\..\..\..\driver\xp

pause

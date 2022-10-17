del *.bak /s
del *.ddk /s
del *.edk /s
del *.lst /s
del *.lnp /s
del *.mpf /s
del *.mpj /s
del *.obj /s
del *.omf /s
::del *.opt /s  ::不允许删除JLINK的设置
del *.plg /s
del *.rpt /s
del *.tmp /s
del *.__i /s
del *.crf /s
del *.o /s
del *.d /s
del *.axf /s
del *.tra /s
del *.dep /s           
del JLinkLog.txt /s

del *.iex /s
del *.htm /s
del *.sct /s
del *.map /s
del *.elf /s
del *.log /s
del *.progstat /s
del *.cache /s

del *.old /s
::del *.bin /s

del *.s
del STM32F401CEUX_RAM.ld
::del .cproject
del .project
del .mxproject
del Makefile

rmdir .\MDK-ARM /s /q

rmdir .\Debug /s /q
rmdir .\Release /s /q

rmdir .\auto-water-change\Debug /s /q
rmdir .\auto-water-change\Release /s /q

rmdir .\.vs /s /q
rmdir .\auto-water-change\.vs /s /q
rmdir .\auto-water-change\CodeDB /s /q
rmdir .\auto-water-change\.visualgdb /s /q


exit

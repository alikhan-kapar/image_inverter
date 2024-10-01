call C:\Xilinx\Vivado\2021.1\bin\vivado.bat -mode batch -source dma.tcl

@echo off
rem Delete the files if they exist
if exist "vivado.jou" del /f /q "vivado.jou"
if exist "vivado.log" del /f /q "vivado.log"

rem Delete the folder and its contents if it exists
if exist ".Xil" rd /s /q ".Xil"

pause
set current_dir=%cd%\..\Graphics\Shaders\Redist
cd %current_dir%

:: CLEAN UP FOLDER FROM PREVIOUS COMPILATION
for %%f in (*.spv) do del %%f

:: CONVERT TO GLSL FILE-EXTENSIONS
for %%f in (*VS.hlsl) do copy /Y /D %%f %%~nf.vert
for %%f in (*HS.hlsl) do copy /Y /D %%f %%~nf.tesc
for %%f in (*DS.hlsl) do copy /Y /D %%f %%~nf.tese
for %%f in (*GS.hlsl) do copy /Y /D %%f %%~nf.geom
for %%f in (*PS.hlsl) do copy /Y /D %%f %%~nf.frag
for %%f in (*CS.hlsl) do copy /Y /D %%f %%~nf.comp

:: COMPILE SPIR-V SHADER FILES
for %%f in (*.vert) do glslangValidator -e main -o %%~nf.spv -V -D %%f
for %%f in (*.tesc) do glslangValidator -e main -o %%~nf.spv -V -D %%f
for %%f in (*.tese) do glslangValidator -e main -o %%~nf.spv -V -D %%f
for %%f in (*.geom) do glslangValidator -e main -o %%~nf.spv -V -D %%f
for %%f in (*.frag) do glslangValidator -e main -o %%~nf.spv -V -D %%f
for %%f in (*.comp) do glslangValidator -e main -o %%~nf.spv -V -D %%f

:: CLEAN-UP
for %%f in (*.vert) do del %%f
for %%f in (*.tesc) do del %%f
for %%f in (*.tese) do del %%f
for %%f in (*.geom) do del %%f
for %%f in (*.frag) do del %%f
for %%f in (*.comp) do del %%f

exit

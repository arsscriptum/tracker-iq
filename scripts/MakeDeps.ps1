
function PrintTitleString {
    param (
        $str
    )
    
    $max=120
    $StrSep = [string]::new('=',$max)
    $l = $str.Length 
    $l = if ($l % 2 -ne 0) { $l + 1 } else { $l }

    $index = [math]::Abs(($max/2)-($l/2))
    $Spaces = [string]::new(' ',$index)
    $TitleStr = "{0}{1} {0}" -f $Spaces,$str,$Spaces

    Write-Host "`n╔$StrSep╗`n║" -f DarkRed -NoNewLine
    Write-Host "$TitleStr" -f Yellow -NoNewLine
    Write-Host "║`n╚$StrSep╝`n`n" -f DarkRed
}



function GetSetupConfigurations {
    param (
        $whereArgs
    )
    
    Invoke-Expression "& $VsWherePath $whereArgs -format json" | ConvertFrom-Json
}

function ImportDevShell {
    param ()

    $basePath =  "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community"


    $currModulePath = "$basePath\Common7\Tools\Microsoft.VisualStudio.DevShell.dll"
    # Prior to 16.3 the DevShell module was in a different location
    $prevModulePath = "$basePath\Common7\Tools\vsdevshell\Microsoft.VisualStudio.DevShell.dll"

    $modulePath = if (Test-Path $prevModulePath) { $prevModulePath } else { $currModulePath }

    if (Test-Path $modulePath) {
        Write-Verbose "Found at $modulePath."

        try {
            Import-Module $modulePath -PAssthru

        }
        catch [System.IO.FileLoadException] {
            Write-Verbose "The module has already been imported from a different installation of Visual Studio:"
            (Get-Module Microsoft.VisualStudio.DevShell).Path | Write-Verbose
        }


    }

}

try{

    Write-Host "Checking VisualStudio Installation..." -f  DarkYellow -NoNewLine
    $VisualStudio2019Community = "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community" 
    if(-not(Test-Path -Path "$VisualStudio2019Community")){ throw "missing VisualStudio Community" }

    Write-Host "OK" -f DarkGreen

    Write-Host "RootPath " -f  DarkYellow -NoNewLine
    $RootPath = (Resolve-Path -Path "$PSScriptRoot\..").Path
    Write-Host "$RootPath" -f DarkGreen

    Write-Host "ExternalPath " -f  DarkYellow -NoNewLine
    $ExternalPath = (Resolve-Path -Path "$RootPath\external").Path
    Write-Host "$ExternalPath" -f DarkGreen

    Write-Host "LibTorrentPath " -f  DarkYellow -NoNewLine
    $LibTorrentPath = (Resolve-Path -Path "$ExternalPath\libtorrent-iq").Path
    Write-Host "$LibTorrentPath" -f DarkGreen

    Write-Host "ProjectPath " -f  DarkYellow -NoNewLine
    $ProjectPath = (Resolve-Path -Path "$RootPath\vs").Path
    Write-Host "$ProjectPath" -f DarkGreen

    Write-Host "CMAKE " -f  DarkYellow -NoNewLine
    $CmakeCmd = get-command 'cmake.exe' -ErrorAction  Ignore  
    if($Null -eq $CmakeCmd){ throw "missing cmake" }


    $CmakeExe = $CmakeCmd.Source
    Write-Host "$CmakeExe" -f DarkGreen


    Write-Host "Getting Submodules..." -f  DarkYellow -NoNewLine
    git submodule update --init --recursive
    Write-Host "OK" -f DarkGreen

    Write-Host "LibSignalPath " -f  DarkYellow -NoNewLine
    $LibSignalPath = (Resolve-Path -Path "$LibTorrentPath\deps\try_signal").Path
    Write-Host "$LibSignalPath" -f DarkGreen


    Push-Location "$LibSignalPath"
    &"$CmakeExe" .
    if($LASTEXITCODE -ne 0){ throw "error  when running cmake" }
    Write-Host "OK Exit Code: $LASTEXITCODE" -f DarkGreen
    Write-Host "LibSignalSlnPath " -f  DarkYellow -NoNewLine
    $LibSignalSlnPath = (Resolve-Path -Path "$LibSignalPath\try_signal.sln").Path
    Write-Host "$LibSignalSlnPath" -f DarkGreen
    Pop-Location

    Push-Location "$LibTorrentPath"
    Write-Host "Building Projects Files" -f  DarkYellow -NoNewLine
    &"$CmakeExe" .

    if($LASTEXITCODE -ne 0){ throw "error  when running cmake" }
    Write-Host "OK Exit Code: $LASTEXITCODE" -f DarkGreen
    Pop-Location

    Write-Host "LibTorrentSlnPath " -f  DarkYellow -NoNewLine
    $LibTorrentSlnPath = (Resolve-Path -Path "$LibTorrentPath\libtorrent.sln").Path
    Write-Host "$LibTorrentSlnPath" -f DarkGreen

    Push-Location "$LibSignalPath"

    $Configuration = "Release"
    ImportDevShell
    Enter-VsDevShell -VsInstallPath "$VisualStudio2019Community"




    Clear-Host

    $MSBuildCmd = get-command 'msbuild.exe' -ErrorAction  Ignore  
    if($Null -eq $MSBuildCmd){ throw "missing cmake" }

    $MSBuildExe = $MSBuildCmd.Source

    PrintTitleString "Building $Configuration $LibSignalSlnPath"

    &"$MSBuildExe" "$LibSignalSlnPath" "-m" "-t:Build" "-p:Configuration=$Configuration"

    Write-Host "SignalBinLibPath " -f  DarkYellow -NoNewLine
    $SignalBinLibPath = (Resolve-Path -Path "$LibSignalPath\$Configuration").Path
    Write-Host "$SignalBinLibPath" -f DarkGreen


    PrintTitleString "Building $Configuration $LibTorrentSlnPath"


    &"$MSBuildExe" "$LibTorrentSlnPath" "-m" "-t:Build" "-p:Configuration=$Configuration"

    Write-Host "TorrentBinLibPath " -f  DarkYellow -NoNewLine
    $TorrentBinLibPath = (Resolve-Path -Path "$LibTorrentPath\$Configuration").Path
    Write-Host "$TorrentBinLibPath" -f DarkGreen

    $InstallPath = Join-Path "$LibTorrentPath" "install"
    $RelLibs = Join-Path "$InstallPath" "MT" 
    

    New-Item -Path "$RelLibs"  -ItemType directory -Force -ErrorAction Stop | Out-Null

    Copy-Item -Path "$SignalBinLibPath\*.lib" -Destination "$RelLibs" -Force -Verbose
    Copy-Item -Path "$TorrentBinLibPath\*.lib" -Destination "$RelLibs" -Force -Verbose

    PrintTitleString "$RelLibs"
    Get-ChildItem -Path "$RelLibs" | Select -ExpandProperty Name

    $Configuration = "Debug"

   
    PrintTitleString "Building $Configuration $LibSignalSlnPath"

    $Configuration = "Debug"
    &"$MSBuildExe" "$LibSignalSlnPath" "-m" "-t:Build" "-p:Configuration=$Configuration"

    Write-Host "SignalBinLibPath " -f  DarkYellow -NoNewLine
    $SignalBinLibPath = (Resolve-Path -Path "$LibSignalPath\$Configuration").Path
    Write-Host "$SignalBinLibPath" -f DarkGreen

    PrintTitleString "Building $Configuration $LibTorrentSlnPath" 

    &"$MSBuildExe" "$LibTorrentSlnPath" "-m" "-t:Build" "-p:Configuration=$Configuration"

    Write-Host "TorrentBinLibPath " -f  DarkYellow -NoNewLine
    $TorrentBinLibPath = (Resolve-Path -Path "$LibTorrentPath\$Configuration").Path
    Write-Host "$TorrentBinLibPath" -f DarkGreen

    $DebugLibs = Join-Path "$InstallPath" "MTd"
    New-Item -Path "$DebugLibs"  -ItemType directory -Force -ErrorAction Stop | Out-Null

    Copy-Item -Path "$SignalBinLibPath\*.lib" -Destination "$DebugLibs" -Force -Verbose
    Copy-Item -Path "$TorrentBinLibPath\*.lib" -Destination "$DebugLibs" -Force -Verbose

    PrintTitleString "$DebugLibs"
    Get-ChildItem -Path "$DebugLibs" | Select -ExpandProperty Name


}catch{

    Write-error "$_"

}
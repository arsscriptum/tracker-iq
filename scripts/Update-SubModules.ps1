#╔════════════════════════════════════════════════════════════════════════════════╗
#║                                                                                ║
#║   UpdateSubModules.ps1                                                         ║
#║                                                                                ║
#╟────────────────────────────────────────────────────────────────────────────────╢
#║   Guillaume Plante <codegp@icloud.com>                                         ║
#║   Code licensed under the GNU GPL v3.0. See the LICENSE file for details.      ║
#╚════════════════════════════════════════════════════════════════════════════════╝

[CmdletBinding(SupportsShouldProcess)]
param (
    [Parameter(Mandatory = $False)]
    [string]$BranchName = "main"
)

<#
.SYNOPSIS
    Updates all &"$GitExe" submodules to track and update from the $BranchName branch.

.DESCRIPTION
    This script:
      - Reads the submodule paths from the .&"$GitExe"modules file.
      - Iterates through each submodule directory.
      - Checks the current branch; if in a detached HEAD or not on "$BranchName" or main, it checks out the $BranchName branch.
      - Pulls the latest changes from the remote "$BranchName" branch.
      - Finally, updates the submodule references in the "$BranchName" or main repository.
      
.NOTES
    Run this script from the root of your &"$GitExe" repository.
    Requires &"$GitExe" to be installed and available on the PATH.

    How it works
    ------------
    1. Parsing Submodules: The script reads the `.&"$GitExe"modules` file and extracts each submodule path from lines like `path = external/sqlite`.

    2. Processing Each Submodule:  
       - The script changes directory into each submodule folder.
       - It ensures the submodule is configured to track the "$BranchName" or main or main branch.
       - It checks whether the current branch is "$BranchName" or main or if the submodule is in a detached HEAD state. If so, it checks out "$BranchName" or main
       - It pulls the latest changes from `origin/main`.

    3. Updating the $BranchName Repository:  
       Finally, it adds updated submodule references and attempts to commit them. If there are no changes, it prints a message accordingly.

#>

$GitCmd = Get-Command 'git.exe' -ErrorAction Stop
$GitExe = $GitCmd.Source

# Ensure .gitmodules exists in the current directory.
$gitmodulesPath = ".gitmodules"
if (!(Test-Path $gitmodulesPath)) {
    Write-Error ".gitmodules not found. Please run this script in the repository root."
    exit 1
}

Write-Host "Parsing .gitmodules for submodule paths..."

# Parse submodule paths from .gitmodules. Assumes lines of the form "   path = <submodule_path>"
$submodulePaths = Select-String -Path $gitmodulesPath -Pattern 'path\s*=' | ForEach-Object {
    ($_ -split "=")[1].Trim()
}

if ($submodulePaths.Count -eq 0) {
    Write-Host "No submodules found in .gitmodules."
    exit 0
}
Write-Host "[Update-SubModules] " -f DarkRed -NoNewLine
Write-Host "Found submodules" -f DarkYellow
Write-Host "-----------------" -f DarkCyan
$submodulePaths | ForEach-Object { Write-Host "  - $_" -f DarkMagenta }

Write-Host "-----------------`n" -f DarkCyan
foreach ($submodule in $submodulePaths) {
    Write-Host "`nProcessing submodule: $submodule" -ForegroundColor Cyan

    if (!(Test-Path $submodule)) {
        Write-Warning "Submodule path '$submodule' does not exist. Skipping."
        continue
    }

    Push-Location $submodule
    &"$GitExe" 'fetch' 
    Write-Host "[Update-SubModules] " -f DarkRed -NoNewLine
    Write-Host "Ensure the submodule is configured to track the $BranchName branch" -f DarkYellow
    $opt = "submodule.{0}.branch" -f $submodule
    &"$GitExe" 'config' "$opt" 'main' | Out-Null

    Write-Host "[Update-SubModules] " -f DarkRed -NoNewLine
    Write-Host "Retrieve the current branch. If in a detached HEAD, this command returns nothing" -f DarkYellow
    [string]$Out = (&"$GitExe" 'symbolic-ref' '--short' '-q' 'HEAD') -as [string]
    $currentBranch = $Out.Trim()

    if ([string]::IsNullOrEmpty($currentBranch)) {
        Write-Host "[Update-SubModules] " -f DarkRed -NoNewLine
        Write-Host "Submodule is in detached HEAD state. Checking out '$BranchName'..."
        &"$GitExe" 'checkout' $BranchName
    }
    elseif ($currentBranch -ne "$BranchName") {
        Write-Host "[Update-SubModules] " -f DarkRed -NoNewLine
        Write-Host "Submodule is on branch '$currentBranch'. Switching to '$BranchName'..."
        &"$GitExe" 'checkout' "$BranchName"
    }
    else {
        Write-Host "[Update-SubModules] " -f DarkRed -NoNewLine
        Write-Host "Submodule is already on branch '$BranchName'."
    }
    Write-Host "[Update-SubModules] " -f DarkRed -NoNewLine
    Write-Host "Pulling latest changes for submodule..."
    &"$GitExe" 'pull' 'origin' "$BranchName"

    Pop-Location
}
Write-Host "[Update-SubModules] " -f DarkRed -NoNewLine
Write-Host "`nUpdating $BranchName repository submodule references..."
&"$GitExe" 'add' '-u'
$commitOutput = &"$GitExe" 'commit' '-m' "Updated submodules to latest $BranchName branch commits"
if ($LASTEXITCODE -eq 0) {
    Write-Host "[Update-SubModules] " -f DarkRed -NoNewLine
    Write-Host "Commit successful:" -ForegroundColor Green
    Write-Host "$commitOutput"
}
else {
    Write-Host "[Update-SubModules] " -f DarkRed -NoNewLine
    Write-Host "No submodule reference changes to commit." -ForegroundColor Yellow
}

Write-Host "[Update-SubModules] " -f DarkRed -NoNewLine
Write-Host "Submodule update complete." -ForegroundColor Green

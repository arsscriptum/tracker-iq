#╔════════════════════════════════════════════════════════════════════════════════╗
#║                                                                                ║
#║   GetRatedTrackers.ps1                                                         ║
#║                                                                                ║
#╟────────────────────────────────────────────────────────────────────────────────╢
#║   Guillaume Plante <codegp@icloud.com>                                         ║
#║   Code licensed under the GNU GPL v3.0. See the LICENSE file for details.      ║
#╚════════════════════════════════════════════════════════════════════════════════╝

[CmdletBinding(SupportsShouldProcess)]
param (
    [Parameter(Mandatory = $False)]
    [switch]$Update
)



$RatingScript = {
    param($Url,$DataDirectory)

    try{
        $RootPath = (Resolve-Path "..").Path
        $TrackerExe = "$RootPath\vs\bin\x64\Release\tracker-rank.exe" 
        if(-not (Test-Path $TrackerExe)){ throw "no binary" }
        $Out = &"$TrackerExe" '--json' '-n' '-u' "$Url"
        $str = ((New-Guid).Guid -as [string]).Substring(30)
        $FilePath = Join-Path "$DataDirectory" "$str"
        $FilePath += '.json'
        
        New-Item -Path "$FilePath" -ItemType file -Value "$Out" -Force | Out-Null
        Write-Output "Wrote $FilePath"
    }catch{
        Show-ExceptionDetails $_ -ShowStack
    }finally{
        Write-verbose "rated $Url"
}}.GetNewClosure()

[scriptblock]$RatingScriptBlock = [scriptblock]::create($RatingScript) 


function Wait-AllJobs {
    # Get all running jobs
    $jobs = Get-Job | Where-Object { $_.State -eq 'Running' }
    
    if ($jobs.Count -eq 0) {
        Write-Host "No running jobs to wait for." -ForegroundColor Green
        return
    }

    Write-Host "Waiting for jobs to complete..." -ForegroundColor Yellow -NoNewLine

    # Wait for jobs to finish
    foreach ($job in $jobs) {
        $jobres = Wait-Job -Id $job.Id
    }


    Write-Host "All jobs completed." -ForegroundColor Green
}

function Get-DateTimeDirectoryName {
    # Define the format for the directory name
    $format = "yyyy-MM-dd_HH-mm-ss"

    # Get the current date and time in the specified format
    $directoryName = Get-Date -Format $format

    # Return the formatted directory name
    return $directoryName
}

function Get-RatedTrackers{
    [CmdletBinding(SupportsShouldProcess)]
    param (
        [Parameter(Mandatory = $False)]
        [string]$Path,
        [Parameter(Mandatory = $False)]
        [switch]$Force,
        [Parameter(Mandatory = $False)]
        [switch]$Asynchronous
    )
    try{
        $RootPath = "D:\Dev\tracker-rank"
        $BinPath = (Resolve-Path "$RootPath\vs\bin\x64\Release" -ErrorAction Stop).Path
        $TrackerExe = Join-Path $BinPath 'tracker-rank.exe'
        $DataDirectory = (Resolve-Path "$RootPath\data" -ErrorAction Stop).Path

        if(Test-Path "$Path"){
            Write-Host "File `"$Path`" already exists." -NoNewLine -f DarkCyan
            if($Force){
                Write-Host "overwriting" -f DarkYellow
                New-Item -Path "$Path" -ItemType file -Force | Out-Null
            }else{
                Write-Host "exiting." -f DarkRed
            }
        }

        if($Asynchronous){
             Get-Job | Remove-Job -Force
             $DataDirectory = Join-Path $DataDirectory (Get-DateTimeDirectoryName)
             New-Item -Path "$DataDirectory" -ItemType "directory" -Force -ErrorAction Ignore | Out-Null
            Write-Host "DataDirectory $DataDirectory" -f DarkYellow
        }

        [System.Collections.ArrayList]$TrackersResults = [System.Collections.ArrayList]::new()
        $TmpFile = "$ENV:Temp\trackers.txt"
        $ProgressPreference ='SilentlyContinue'
        $Url = "https://raw.githubusercontent.com/ngosang/trackerslist/refs/heads/master/trackers_all_ip.txt"

        $Res = Invoke-WebRequest -Uri $Url -SkipCertificateCheck -OutFile $TmpFile -Passthru
        $StatusCode = $Res.StatusCode
        if($StatusCode -ne 200){ throw "Error $_" }
        [string[]]$Lines = Get-Content -Path "$TmpFile"
        $LinesCount = 0
        if(Test-Path "$TrackerExe"){
            $index=0
            ForEach($line in $Lines){
                if([string]::IsNullOrEmpty($line)) { continue; }
                $LinesCount++
            }
            Write-Host "Start rating trackers ($LinesCount total)" -f DarkCyan
            ForEach($line in $Lines){
                if([string]::IsNullOrEmpty($line)) { continue; }
                $index++
               
                if($Asynchronous){
                    $JobName = ((New-Guid).Guid -as [string]).Substring(30)
                    $jobby = Start-Job -Name $JobName -ScriptBlock $RatingScriptBlock -ArgumentList ($line,$DataDirectory)
                    $strlog = "[{0:d3}/{1:d3}] testing `"{2}`". job is {3}" -f $index,$LinesCount,$line,$JobName 
                    Write-Host $strlog -f DarkCyan
                }else{
                    Write-Verbose "testing `"$line`" "
                    $Out = &"$TrackerExe" '--json' '-n' '-u' "$line"
                    $Data = $Out | ConvertFrom-Json
                    [void]$TrackersResults.Add($Data)
                }
            }
        }else{
            throw "missing tracker-rank.exe binary"
        }
        if($Asynchronous){
            Wait-AllJobs
            Write-Host "parsing json files..."
            $AllJsonFiles = Get-ChildItem -Path "$DataDirectory" -Filter "*.json"
            ForEach($file in $AllJsonFiles.Fullname){
                $JsonItem = Get-Content $file | ConvertFrom-Json
                [void]$TrackersResults.Add($JsonItem)
            }
        }
        $AllData = $TrackersResults | ConvertTo-Json
        Write-Host "saving to `"$Path`""
        Set-Content -PAth "$Path" -Value "$AllData"
        $TrackersResults 
    }catch{
        Write-Error "$_"
    }
}



if($Update){
    $trackers = Get-RatedTrackers -Verbose -Path "d:\Tmp\trackers.json" -Force
}

$trackers = Get-Content "d:\Tmp\trackers.json" | ConvertFrom-Json
# Define weights for peers and response_time
$Weight_Peers = 0.7
$Weight_response_time = 0.3

# Find Min and Max values for normalization
$MinPeers = ($trackers | Measure-Object -Property num_peers -Minimum).Minimum
$MaxPeers = ($trackers | Measure-Object -Property num_peers -Maximum).Maximum
$response_time_min = ($trackers | Measure-Object -Property response_time -Minimum).Minimum
$response_time_max = ($trackers | Measure-Object -Property response_time -Maximum).Maximum

# Function to normalize a value
function Normalize-Value {
    param (
        [float]$Value,
        [float]$Min,
        [float]$Max
    )
    if ($Max -eq $Min) {
        return 1.0 # Avoid division by zero, assume normalized value of 1
    }
    return ($Value - $Min) / ($Max - $Min)
}

$trackers = $trackers | where is_valid -eq $True | where num_peers -ge 1

# Calculate quality score for each tracker
$trackers | ForEach-Object {
    $NormalizedPeers = Normalize-Value -Value $_.num_peers -Min $MinPeers -Max $MaxPeers
    $Normalizedresponse_time = Normalize-Value -Value $_.response_time -Min $response_time_min -Max $response_time_max

    # Calculate quality score
    $QualityScore = ($Weight_Peers * $NormalizedPeers) - ($Weight_response_time * $Normalizedresponse_time)

    # Add the quality score to the object
    $_ | Add-Member -MemberType NoteProperty -Name quality_score -Value $QualityScore
}

# Display the results sorted by QualityScore
$trackers | Sort-Object -Property quality_score -Descending | ForEach-Object {
    [PSCustomObject]@{
        url          = $_.url
        is_valid      = $_.is_valid
        num_peers     = $_.num_peers
        response_time      = $_.response_time
        last_tested  = $_.last_tested
        quality_score = $_.quality_score
    }
}



# Calculate the average response_time and NumPeers
$Averageresponse_time = ($trackers | Measure-Object -Property response_time -Average).Average
$AverageNumPeers = ($trackers | Measure-Object -Property num_peers -Average).Average

# Filter the entries that are above the average NumPeers and below the average response_time
$aboveAverageTrackers = $trackers | Where-Object num_peers -gt 1

$results = $aboveAverageTrackers | Sort-Object -Property quality_score -Descending

$ResultsPath = (Resolve-Path "$PSScriptRoot\..\data").Path
New-Item -Path "$ResultsPath" -ItemType directory -Force | Out-Null

$ResultsFile = Join-Path $ResultsPath "results.json"

Write-Output "results"
$aboveAverageTrackers | Sort-Object -Property quality_score -Descending

New-Item -Path "$ResultsFile" -ItemType file -Force | Out-Null
Set-Content -Path "$ResultsFile" -Value $results





# Output the calculated averages for reference
Write-Host "Average response_time: $Averageresponse_time ms"
Write-Host "Average NumPeers: $AverageNumPeers"

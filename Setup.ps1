# This script serves:
# - Download and unzip large contents.
# - Download and build third-party libraries.

# Run this script with execution policy like this:
# > powershell -ExecutionPolicy Bypass -File Setup.ps1

#
# Script arguments
#
Param (
	[Switch]$skipdownload,
	[Switch]$skipbuild
)

# For test
$dry_run = $false

$should_download = !($PSBoundParameters.ContainsKey('skipdownload'))
$should_build = !($PSBoundParameters.ContainsKey('skipbuild'))

#
# Constants
#
$thirdparty_dir = "$pwd/thirdparty"
$content_dir    = "$pwd/resources_external"

$contents_list  = @(
	# Format: (url, target_name, unzip_name)
	# - base directory is content_dir.
	# - unzip_name is ignored if the content is not a .zip file.
	@(
		'https://github.com/NVIDIAGameWorks/SpatiotemporalBlueNoiseSDK/raw/main/STBN.zip',
		'STBN.zip',
		'NVidiaSpatioTemporalBlueNoise'
	),
	@(
		"https://casual-effects.com/g3d/data10/research/model/fireplace_room/fireplace_room.zip",
		"fireplace_room.zip",
		"fireplace_room"
	),
	@(
		'https://casual-effects.com/g3d/data10/research/model/breakfast_room/breakfast_room.zip',
		'breakfast_room.zip',
		'breakfast_room'
	),
	@(
		'https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/sourceModels/RiggedFigure/RiggedFigure.dae',
		'KhronosGroup/RiggedFigure/RiggedFigure.dae',
		''
	),
	@(
		'https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/DamagedHelmet/glTF/DamagedHelmet.bin',
		'KhronosGroup/DamagedHelmet/DamagedHelmet.bin',
		''
	),
	@(
		'https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/DamagedHelmet/glTF/DamagedHelmet.gltf',
		'KhronosGroup/DamagedHelmet/DamagedHelmet.gltf',
		''
	),
	@(
		'https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/DamagedHelmet/glTF/Default_AO.jpg',
		'KhronosGroup/DamagedHelmet/Default_AO.jpg',
		''
	),
	@(
		'https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/DamagedHelmet/glTF/Default_albedo.jpg',
		'KhronosGroup/DamagedHelmet/Default_albedo.jpg',
		''
	),
	@(
		'https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/DamagedHelmet/glTF/Default_emissive.jpg',
		'KhronosGroup/DamagedHelmet/Default_emissive.jpg',
		''
	),
	@(
		'https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/DamagedHelmet/glTF/Default_metalRoughness.jpg',
		'KhronosGroup/DamagedHelmet/Default_metalRoughness.jpg',
		''
	),
	@(
		'https://raw.githubusercontent.com/KhronosGroup/glTF-Sample-Models/master/2.0/DamagedHelmet/glTF/Default_normal.jpg',
		'KhronosGroup/DamagedHelmet/Default_normal.jpg',
		''
	),
	@(
		'https://casual-effects.com/g3d/data10/research/model/lpshead/lpshead.zip',
		'LPSHead.zip',
		'LPSHead'
	),
	@(
		'https://casual-effects.com/g3d/data10/research/model/sportsCar/sportsCar.zip',
		'SportsCar.zip',
		'SportsCar'
	),
	@(
		'https://casual-effects.com/g3d/data10/research/model/scrubPine/pine.zip',
		'ScrubPineTree.zip',
		'ScrubPineTree'
	),
	@(
		'https://www2.cs.uic.edu/~i101/SoundFiles/ImperialMarch60.wav',
		'rhythm_game/music_imperial_march.wav',
		''
	),
	@(
		'https://www2.cs.uic.edu/~i101/SoundFiles/StarWars60.wav',
		'rhythm_game/music_star_wars.wav',
		''
	)
)

#
# Utilities
#
function Not-A-Drill {
	return $dry_run -eq $false
}
function Download-URL {
	Param ($webclient, $target_url, $target_path)
	if (Test-Path $target_path -PathType Leaf) {
		Write-Host "   ", $target_path, "already exists, will be skipped." -ForegroundColor Green
	} else {
		if (Not-A-Drill) {
			#Write-Host "url: ", $target_url -ForegroundColor Green
			#Write-Host "target: ", $target_path -ForegroundColor Green
			#Write-Host "Downloading..." -ForegroundColor Green
			$webclient.DownloadFile($target_url, $target_path)
			#Write-Host "Download finished."
		}
	}
}
function Clear-Directory {
	Param ($dir)
	Write-Host "Clear directory:", $dir -ForegroundColor Green
	if (Not-A-Drill) {
		if (Test-Path $dir) {
			Remove-Item -Recurse -Force $dir
		}
		New-Item -ItemType Directory -Force -Path $dir
	}
}
function Ensure-Subdirectory {
	Param ($dir)
	Write-Host "Ensure directory:", $dir
	if (Not-A-Drill) {
		New-Item -ItemType Directory -Path $dir -Force | Out-Null
	}
}
function Unzip {
	Param ($zip_filepath, $unzip_dir)
	Write-Host "    Unzip to:", $unzip_dir
	if (Not-A-Drill) {
		if (Test-Path $unzip_dir) {
			Remove-Item -Recurse -Force $unzip_dir
		}
		Expand-Archive -Path $zip_filepath -DestinationPath $unzip_dir
	}
}

#
# Download contents
#
$num_contents = $contents_list.length
if ($should_download) {
	Write-Host "Download contents... (count=$num_contents)" -ForegroundColor Green
	
	Ensure-Subdirectory "$content_dir"
	$webclient = New-Object System.Net.WebClient
	
	foreach ($desc in $contents_list) {
		$content_url = $desc[0]
		$content_zip = $desc[1]
		$content_unzip = $desc[2]
		Write-Host ">" $content_zip "(" $content_url ")" #-ForegroundColor Green
		
		$zip_path = "$content_dir/$content_zip"
		$unzip_path = "$content_dir/$content_unzip"
		Ensure-Subdirectory ([IO.Path]::GetDirectoryName($zip_path))
		Download-URL $webclient $content_url $zip_path
		
		$file_ext = [IO.Path]::GetExtension($zip_path)
		if ($file_ext -eq ".zip") {
			Unzip $zip_path $unzip_path
		}
	}
	
	# TODO: How to close connection
	#$webclient.Close()
} else {
	Write-Host "Skip download due to -skipdownload" -ForegroundColor Green
}

#
# Unzip libraries
#
if ($should_build) {
	Write-Host "Unzip bass" -ForegroundColor Green
	
	Unzip "$thirdparty_dir/bass/bass24.zip" "$thirdparty_dir/bass/build"
}

<#
$freeimage_url      = "http://downloads.sourceforge.net/freeimage/FreeImage3180.zip"
$freeimage_path     = "$external_dir/FreeImage.zip"

#
# Find MSBuild.exe and devenv.exe
# https://stackoverflow.com/questions/328017/path-to-msbuild
$vswhere_path = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$msbuild_path = &$vswhere_path -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe
#$msbuild_path = "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"
Write-Host "msbuild.exe:", $msbuild_path -ForegroundColor Green
$devenv_path = &$vswhere_path -latest | Select-String -Pattern 'productPath' -SimpleMatch
$devenv_path = $devenv_path.ToString().Substring(13)
Write-Host "devenv.exe:", $devenv_path -ForegroundColor Green

#
# Build third party libraries
#
if ($should_build) {
	### FreeImage
	# 1. Unzip
	if (Test-Path "$external_dir/FreeImage") {
		Write-Host "FreeImage already unzipped. skip unzip."
	} else {
		Write-Host "Unzip FreeImage"
		Expand-Archive -Path $freeimage_path -DestinationPath $external_dir
		Write-Host "Unzip done"
	}
	# 2. Build
	Write-Host "Build FreeImage (x64 | release)"
	# TODO: Rebuild is always triggered after introducing devenv upgrade
	& $devenv_path ./external/FreeImage/FreeImage.2017.sln /Upgrade
	& $msbuild_path ./external/FreeImage/FreeImage.2017.sln -t:FreeImage -p:Configuration=Release -p:Platform=x64 -p:WindowsTargetPlatformVersion=10.0
	# 3. Copy
	$freeimage_source_dir = "./thirdparty/FreeImage/source"
	$freeimage_binary_dir = "./thirdparty/FreeImage/binaries"
	Clear-Directory $freeimage_source_dir
	Clear-Directory $freeimage_binary_dir
	Write-Host "Copy source to $freeimage_source_dir"
	Write-Host "Copy binaries to $freeimage_binary_dir"
	Copy-Item "./external/FreeImage/Dist/x64/FreeImage.h" -Destination $freeimage_source_dir
	Copy-Item "./external/FreeImage/Dist/x64/FreeImage.dll" -Destination $freeimage_binary_dir
	Copy-Item "./external/FreeImage/Dist/x64/FreeImage.lib" -Destination $freeimage_binary_dir
}

Write-Host "> Done" -ForegroundColor Green
#>
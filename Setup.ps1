
#
# Script arguments
#

Param (
	[Switch]$skipdownload,
	[Switch]$skipbuild
)

$should_download = !($PSBoundParameters.ContainsKey('skipdownload'))
$should_build = !($PSBoundParameters.ContainsKey('skipbuild'))

#
# Constants
#
$thirdparty_dir = "$pwd/thirdparty"
$content_dir    = "$pwd/resources_external"

$contents_list  = @(
	# Format: (url, zip_name, unzip_name)
	,@(
		'https://casual-effects.com/g3d/data10/research/model/breakfast_room/breakfast_room.zip',
		'breakfast_room.zip',
		'breakfast_room'
	)
)

#
# Utilities
#
function Download-URL {
	Param ($webclient, $target_url, $target_path)
	if (Test-Path $target_path -PathType Leaf) {
		Write-Host $target_path, "already exists, will be skipped." -ForegroundColor Green
	} else {
		#Write-Host "url: ", $target_url -ForegroundColor Green
		#Write-Host "target: ", $target_path -ForegroundColor Green
		#Write-Host "Downloading..." -ForegroundColor Green
		$webclient.DownloadFile($target_url, $target_path)
		#Write-Host "Download finished."
	}
}
function Clear-Directory {
	Param ($dir)
	Write-Host "Clear directory:", $dir -ForegroundColor Green
	if (Test-Path $dir) {
		Remove-Item -Recurse -Force $dir
	}
	New-Item -ItemType Directory -Force -Path $dir
}
function Ensure-Subdirectory {
	Param ($dir)
	New-Item -ItemType Directory -Path $dir -Force
}
function Unzip {
	Param ($zip_filepath, $unzip_dir)
	if (Test-Path $unzip_dir) {
		Remove-Item -Recurse -Force $unzip_dir
	}
	Expand-Archive -Path $zip_filepath -DestinationPath $unzip_dir
}

#
# Download contents
#
$num_contents = $contents_list.length
if ($should_download) {
	Ensure-Subdirectory "$content_dir"
	$webclient = New-Object System.Net.WebClient
	
	Write-Host "Download contents... (count=$num_contents)" -ForegroundColor Green
	
	foreach ($desc in $contents_list) {
		$content_url = $desc[0]
		$content_zip = $desc[1]
		$content_unzip = $desc[2]
		Write-Host ">" $content_zip "(" $content_url ")" #-ForegroundColor Green
		
		$zip_path = "$content_dir/$content_zip"
		$unzip_path = "$content_dir/$content_unzip"
		Download-URL $webclient $content_url $zip_path
		Unzip $zip_path $unzip_path
	}
	
	# TODO: How to close connection
	#$webclient.Close()
} else {
	Write-Host "Skip download due to -skipdownload" -ForegroundColor Green
}


<#
$freeimage_url      = "http://downloads.sourceforge.net/freeimage/FreeImage3180.zip"
$freeimage_path     = "$external_dir/FreeImage.zip"
$tinyobjloader_url  = "https://github.com/syoyo/tinyobjloader/archive/v2.0.0-rc1.zip"
$tinyobjloader_path = "$external_dir/tinyobjloader-v2.0.0-rc1.zip"
$content_url        = "https://casual-effects.com/g3d/data10/research/model/bedroom/bedroom.zip"
$content_zip_path   = "$external_dir/content/bedroom.zip"
$content_unzip_path = "$content_dir/bedroom/"

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
	### Content
	if (Test-Path "$content_dir/bedroom") {
		Write-Host "Content 'bedroom' already unzipped and will be skipped."
	} else {
		Ensure-Subdirectory "$content_dir/bedroom"
		Write-Host "Unzip content 'bedroom'"
		Expand-Archive -Path $content_zip_path -DestinationPath $content_unzip_path
		Write-Host "Unzip done"
	}
	
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
	
	### tinyobjloader
	# 1. Unzip
	if (Test-Path "$external_dir/tinyobjloader-2.0.0-rc1") {
		Write-Host "tinyobjloader already unzipped. skip unzip."
	} else {
		Write-Host "Unzip tinyobjloader"
		Expand-Archive -Path $tinyobjloader_path -DestinationPath $external_dir
		Write-Host "Unzip done"
	}
	# 2. Copy
	$tinyobjloader_source_dir = "./thirdparty/tinyobjloader/source"
	Clear-Directory $tinyobjloader_source_dir
	Write-Host "Copy source to $tinyobjloader_source_dir"
	Copy-Item "./external/tinyobjloader-2.0.0-rc1/tiny_obj_loader.h" -Destination $tinyobjloader_source_dir
	Copy-Item "./external/tinyobjloader-2.0.0-rc1/tiny_obj_loader.cc" -Destination $tinyobjloader_source_dir
}

Write-Host "> Done" -ForegroundColor Green
#>
# $date_str = Get-Date -Format "yyyy-MM-dd"
# $release_dir = ("builds/{0}" -f $date_str)
$release_dir = "builds/starfield_navigator"

if (Test-Path $release_dir) {
   Remove-Item -Path $release_dir -Recurse
}
New-Item -Path "." -Name $release_dir -ItemType "directory" | Out-Null

$name = "starfield_navigator"
Copy-Item ("x64/Release/{0}.exe" -f $name) -Destination $release_dir
Copy-Item ("{0}/system_data.txt" -f $name) -Destination $release_dir
Copy-Item ("{0}/cc_hyg.txt" -f $name) -Destination $release_dir
Copy-Item ("{0}/shaders" -f $name) -Destination $release_dir -Recurse

$zip_path = "builds/starfield_navigator.zip"
if (Test-Path $zip_path) {
   Remove-Item $zip_path
}
Compress-Archive -Path $release_dir -DestinationPath $zip_path
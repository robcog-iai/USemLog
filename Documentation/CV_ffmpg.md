run the following shell script in \Scans\. folder:

```
$fps_array = @(5, 25)
$array = @("D", "L", "M", "N", "U", "A")
Get-ChildItem -Directory | ForEach-Object{  
    foreach ($element in $array) {
        foreach ($fps_element in $fps_array) {            
            $VidPath = "$($_.Name)/$($_.Name)_$($element)_fps$($fps_element).mp4"
            $ImgPath = "$($_.Name)/$($element)/img1%04d.png"
            ffmpeg -y -framerate $fps_element -i $ImgPath $VidPath
        }
    }
}
```


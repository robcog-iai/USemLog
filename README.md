![](Documentation/Img/SemLog.png)

# USemLog

Semantic logging plugin for Unreal Engine. Logs symbolic and sub-symbolic data to a KnowRob compatible format.
	
# Usage:

#### Include the plugin to your project
* Add the plugin to your project (e.g `MyProject/Plugins/USemLog`)

#### Tag semantic components (see [UTags](https://github.com/robcog-iai/UTags) for more details)

* Tag your actors / components that you want to semantically log:

  * Tag example:

    [`SemLog;Class,HelaCurryKetchup;Runtime,Dynamic;Id,gPP9;`]

    where

     `SemLog;` is the `TagType`

     `Class,HelaCurryKetchup;` - represents the semantic class of the object

     `LogType,Dynamic;` - represents the raw logging type of the object (static or dynamic)

     `Id,gPP9;` - represents the unique ID for each entity to be logged

# Rules:

 * All meshes/items/actor/components need to have the scale set to `1,1,1` in Unreal
 * The center of the objects should be in the center of bounds.



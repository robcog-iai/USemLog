![](Documentation/Img/SemLog.png)

# USemLog

Semantic logging plugin for Unreal Engine. Logs symbolic and sub-symbolic data to a KnowRob compatible format.
	
# Usage:

#### Include the plugin to your project
* Add the plugin to your project (e.g `MyProject/Plugins/USemLog`)

#### Tag semantic components (see [UTags](https://github.com/robcog-iai/UUtils/tree/master/Source/UTags) for more details)

* Tag your actors / components that you want to semantically log:

  * Tag example:

    [`SemLog;SubClassOf,KetchupBottle;Class,HelaCurryKetchup;Mobility,Dynamic;Id,gPP9;`]

    where

     `SemLog;` is the `TagType`

	 `SubClassOf,KetchupBottle;` - represents the parent of the semantic class
	 
     `Class,HelaCurryKetchup;` - represents the semantic class of the object

     `Mobility,Dynamic;` - represents the raw logging type of the object (Static or Dynamic)
	 
     `Id,y6dnf3eQsUK9pPKAzo90yA;` - unique id in UUID in Base64Url



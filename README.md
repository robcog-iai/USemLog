![](Documentation/Img/SemLog.png)

# USemLog

Semantic logging plugin for Unreal Engine. Logs symbolic and sub-symbolic data to a KnowRob compatible format.

# Citations

```bibtex
@inproceedings{haiduameva19,
  author    = {Andrei Haidu, Michael Beetz},
  title     = {Automated Models of Human Everyday Activity based on Game and Virtual Reality Technology},
  booktitle = {International Conference on Robotics and Automation (ICRA)},
  year      = {2019}
}
```

```bibtex
@inproceedings{DBLP:conf/iros/HaiduBBB18,
  author    = {Andrei Haidu, Daniel Bessler, Asil Kaan Bozcuoglu, Michael Beetz},
  title     = {KnowRob_SIM - Game Engine-Enabled Knowledge Processing Towards Cognition-Enabled Robot Control},
  booktitle = {2018 {IEEE/RSJ} International Conference on Intelligent Robots and Systems, {IROS} 2018, Madrid, Spain, October 1-5, 2018},
  year      = {2018},
  url       = {https://doi.org/10.1109/IROS.2018.8593935}
}
```

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

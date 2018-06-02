# Installing binaries

This should rarely be needed, as the current versions of our modules are in the plugin manager.

Download the current release from our [releases page](https://github.com/squinkylabs/SquinkyVCV/releases), or get them from someone who built them for you. Warning: often we do not post all the current releases.

Follow the standard instructions for installing third-party plugins in rack: [here](https://vcvrack.com/manual/Installing.html)

Note that the built-in unzip in windows explorer will create an extra folder called with the same name as the zip file. But when everything is unzipped you want your folder structure to look like this:
```
plugins/
    <other plugins>
    squinkylabs-plug1/
        plugin.dll
        plugin.dylib  (optional, unused)
        plugin.so     (optional, unused)
        LICENSE
        res/   
            <one or more svg files>
            <possibly other resources>
```

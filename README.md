# MeSH-JSON
Lightning-fast [MeSH (NCBI Medical Subject Headings)](https://www.ncbi.nlm.nih.gov/mesh) database [bulk data](https://www.nlm.nih.gov/databases/download/mesh.html) reader &amp; JSON lines converter.

## Just want the data?

Download here: [https://techoverflow.net/downloads/desc2020.json.gz](https://techoverflow.net/downloads/desc2020.json.gz)
or
```sh
wget "https://techoverflow.net/downloads/desc2020.json.gz"
```

The format is a GZ-compressed JSON-Lines format, i.e. one JSON (representing one MeSH descriptor) per line.

## How to use (manual mode)

First, download the current MeSH dump in XML format from the [bulk data](https://www.nlm.nih.gov/databases/download/mesh.html) download page - for example, `desc2020.xml`.

Secondly, clone & compile MeSH-JSON. You need to install [rapidjson-dev](https://rapidjson.org/) and [PugiXML](https://pugixml.org/) and CMake plus the usual G++ stuff, e.g. using `sudo apt install -y rapidjson-dev libpugixml-dev cmake build-essential` on Ubuntu):
```sh
git clone https://github.com/ulikoehler/MeSH-JSON.git
cd MeSH-JSON
cmake .
make
```

Now you can run *MeSH-JSON*:
```
./mesh-json ~/Downloads/desc2020.xml ~/Downloads/desc2020.json
```

*MeSH-JSON* will just take a couple of seconds to process the XML and convert it into a JSON containing roughly the same information. In the current version, some metadata like the date of the records isn't parsed (feel free to add it and submit a pull request).

## Rationale

MeSH provides bulk data as XML format (amongst some other formats). However the XML file is somewhat large and I have seen many developers struggle with the size of the dataset (~300 Megabytes for the 2020 dump).

The XML format seems to be the most straightforward and complete of all the available formats. For example, the ASCII format (for which I have published [a blogpost on parsing it using Python](https://techoverflow.net/2015/02/18/parsing-the-mesh-ascii-format-in-python/) back in 2015) contains only abbreviated fields and does not seem to include all the information from the XML.

However, if you just try to parse the entire file in Python, e.g. using `lxml`, this will consume large amount of memory (about 4 GB). Also, parsing the XML data is very inconvenient as compared to more standard JSON-like data and since parsing MeSH is just a means to an end to achieve some higher-order goal, bothering with the XML format is just a waste of time for many computational biologists.

Additionally, many developers consider memory and CPU *too cheap to bother* and hence don't care about optimization whatsoever. Part of this is due to lack of skill and experience in optimizing an application without either making it too complex or slowing down the development too much. In my opinion, much of this issue stems from the lack of appropriate copy-to-paste-ready examples that use faster technology (e.g. PugiXML & C++ instead of LXML & Python for the raw parsing work), but in case such examples exist, it is often worthwhile to do it in a more efficient way to prevent secondary issues that occur only later in the development process.

Hence, the *MeSH-JSON* project attempts to:
* Provide the MeSH datadump in a ready-to-use [JSON-Lines](http://jsonlines.org/) format (one MeSH descriptor per line)
* Provide an example of how large XML datasets can be parsed and processed to JSON easily using PugiXML & RapidJSON
* Provide documentation on common issues encountered along the way on my Blog [TechOverflow](https://techoverflow.net)

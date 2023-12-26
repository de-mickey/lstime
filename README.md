# lstime

## Description
`lstime` command line utility showing various timestamps associated with files.

## Features
The output and timestamp formatting is very flexible:
- Can specify which timestamps to see: mtime, atime, ctime, and/or btime
- Can specify most every detail of the time format, like sub-second precision
- Can select aspects of the pathname quoting and escaping
- Can optionally sort by timestamps or pathname

## Platform
Intended for recent Linux environments.  Written in C.

## License
GNU General Public License, version 3 or later.

## Some Examples
```
$ lstime s*.h    # By default, shows mtime and atime
2021-03-23T19:10:00.932  2023-11-28T23:38:43.694  sha256.h
2021-02-28T11:26:02.112  2023-11-28T23:38:43.706  sncpy.h
2002-01-07T15:18:25.000  2023-11-28T23:38:43.706  strchrdel.h

$ lstime -e strchrdel.h    # show all 4 timestamps with labels
strchrdel.h
    modified  2002-01-07 15:18:25.000000000 -05:00
    accessed  2023-11-28 23:38:43.706443782 -05:00
     changed  2021-03-08 08:44:18.856001778 -05:00
        born  2016-01-25 23:29:42.692673016 -05:00

# use custom formats 
$ lstime --item-format '%c %u%n' --time-format '%c' s*.h
Tue 23 Mar 2021 07:10:00 PM EDT sha256.h
Sat 06 Mar 2021 10:49:25 PM EST sncpy.h
Mon 08 Mar 2021 08:44:18 AM EST strchrdel.h
```


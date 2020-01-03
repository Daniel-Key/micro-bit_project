# CS3099-Server

## Instructions

(This assumes you are not logged into SSH.)
Before running the server for the first time, run:
```
make update-conf
```
and
```
npm install
```

Then, you will need to actually run the server with:
```
make run
```

If this doesn't work due to another process listening on that port, kill the other process with:
```
make kill
```

To view the website, use:
```
make open
```

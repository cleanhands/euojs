euojs
=====

`euojs` is a Node.js addon for loading and calling the OpenEUO uo.dll with
pure javascript. It can be used to create [EasyUO][easyuowebsite] scripts without writing any
LUA or EasyUO code.

It also enables you to use networking and any other Node.js modules in your
EasyUO scripts.


Example
-------

``` js
var client = require('euojs').createClient()

client.CliNr(1, function(err, res) {
  client.monitor('journal')
})

client.on('journal', function(j) {
  console.log('[%d] %s', j.color, j.text)
})
```


Requirements
------------

 * OpenEUO's uo.dll.
 * Node.js v0.8+.
 * Windows


Installation
------------

First you need to install [`node-gyp`](https://github.com/TooTallNate/node-gyp) and it's requirements.

To compile `euojs`:

``` bash
$ git clone git://github.com/cleanhands/euojs.git
$ cd euojs
$ node-gyp configure build
```


Documentation
-------------

See the [OpenEUO wiki][openeuowiki] for documentation. All UO commands and
variables are implemented asynchronously. Pass a value as the first argument to
variable accessors to set it, or a callback to get the current value.

In addition to the standard API `euojs` adds monitor() and stopMonitor() which
may be passed `'all'`, `'journal'`, or an OpenEUO variable name. Monitored
variables emit events when their value changes.


License
-------

MIT License. See the `LICENSE` file.

[openeuowiki]: http://www.easyuo.com/openeuo/wiki/index.php/Documentation
[easyuowebsite]: http://www.easyuo.com/
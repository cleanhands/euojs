var util = require('util')
  , events = require('events')
  , uodll = require('../build/Release/uodll')  
  , variables = require('./variables.js')
  , commands = require('./commands.js')

function UO() {
  this._handle = uodll.getHandle()
  this._queue = []
  this._busy = false
  this._monitors = {}
  this._previousValues = {}
}
util.inherits(UO, events.EventEmitter)

UO.prototype.callDll = function(command, callback) {
  this._queue.push({'command': command, 'callback': callback})
  if(!this._busy) {
    this._busy = true
    this.processQueue()
  }
}

UO.prototype.processQueue = function() {
  var that = this
  if(this._queue.length > 0) {
    var q = this._queue.shift()
    uodll.call(this._handle, q.command, function(res) {
      if(res.length > 0) {
        q.callback(false, res)
      } else {
        q.callback('No result returned from dll.', false)
      }
      that.processQueue()
    })
  } else {
    this._busy = false
  }
}

UO.prototype.monitor = function(type) {
  var that = this
  switch(type) {
    case 'journal':
      that.ScanJournal(0, function(err, res) {
        that.jref = res.ref
        that._monitors[type] = setInterval(function() {
          that.ScanJournal(that.jref, function(err, j) {
            if (j.ref !== that.jref) {
              var loop = function(i) {
                if(i >= 0) {
                  that.GetJournal(i, function(err, m) {
                    that.emit('journal', m)
                  })
                  loop(--i)
                } 
              }
              loop(j.count - 1)
              that.jref = j.ref
            }
          })
        }, 50)
      })
      break
    case 'all':
      variables.forEach(function(e, i, a) {
        that.monitor(e)
      })
      that.monitor('journal')
      break
    default:
      if(variables.indexOf(type) !== -1) {
        that[type](function(err, value) {
          that._previousValues[type] = value
          that._monitors[type] = setInterval(function() {
            that[type](function(err, value) {
              if(that._previousValues[type] !== value) {
                that.emit(type, value, that._previousValues[type])
                that._previousValues[type] = value
              }
            })
          }, 50)
        })
      }
      break
  }
}

UO.prototype.stopMonitor = function(type) {
  if(type === 'all') {
    for(i in this._monitors) {
      clearInterval(this._monitors[i])
    }
  } else if(this._monitors[type]) {
    clearInterval(this._monitors[type])
  }
}

variables.forEach(function(variable, i) {
  var func = function(value, callback) {
    if(typeof value === 'function') {
      var command = ['Get', variable]
      callback = value
    } else {
      var command = ['Set', variable, value]
    }
    this.callDll(command, function(err, res) {
      callback(err, res[0])
    })
  }
  Object.defineProperty(UO.prototype, variable, { value: func, enumerable: true })
})

commands.forEach(function(command, i) {
  var func = function() { 
    var args = ['Call', command]
      , callback
    for(i = 0; i < arguments.length; i++) {
      if(typeof arguments[i] === 'function') {
        callback = arguments[i]
      } else {
        args.push(arguments[i])
      }
    }
    this.callDll(args, function(err, res) {
      if(err) {
        var result = false
      } else if (res.length === 1) {
        var result = res[0]
      } else {
        switch(command) {
          case 'GetCont':
            var result = {
                'name': res[0]
              , 'x': res[1]
              , 'y': res[2]
              , 'width': res[3]
              , 'height': res[4]
              , 'gumpkind': res[5]
              , 'id': res[6]
              , 'type': res[7]
              , 'hp': res[8]
            }
            break
          case 'GetItem':
            var result = {
                'id': res[0]
              , 'type': res[1]
              , 'kind': res[2]
              , 'container_id': res[3]
              , 'x': res[4]
              , 'y': res[5]
              , 'z': res[6]
              , 'reputation': res[7]
              , 'color': res[8]
            }
            break
          case 'GetJournal':
            var result = {
                'text': res[0]
              , 'color': res[1]
            }
            break
          case 'GetShop':
            var result = {
                'result': res[0]
              , 'position': res[1]
              , 'count': res[2]
              , 'id': res[3]
              , 'type': res[4]
              , 'max': res[5]
              , 'price': res[6]
              , 'name': res[7]
            }
            break
          case 'GetSkill':
            var result = {
                'adjusted': res[0]
              , 'real': res[1]
              , 'cap': res[2]
              , 'lock': res[3]
            }
            break
          case 'ScanJournal':
            var result = {
                'ref': res[0]
              , 'count': res[1]
            }
            break
          default:
            var result = res
            break
        }
      }
      callback(err, result)
    })
  }
  Object.defineProperty(UO.prototype, command, { value: func })
})

exports.createClient = function() {
  var client = new UO()
  return client
}
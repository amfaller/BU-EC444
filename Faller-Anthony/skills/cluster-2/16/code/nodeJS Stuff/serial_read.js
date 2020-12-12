/* EC444 Quest02 Skill16
*  Node.js
*  October 1, 2020
*  Author: Tony Faller  */


/* This code taken from https://serialport.io/docs/api-serialport */

const SerialPort = require('serialport')
const Readline = SerialPort.parsers.Readline
const port = new SerialPort('COM3', {baudRate:115200})

const parser = port.pipe(new Readline())
parser.on('data', console.log)


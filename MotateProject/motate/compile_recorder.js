#!/usr/bin/env node
var fs = require("fs")

// Copy the argv
var new_process_args = process.argv.slice(2)

var directory = new_process_args.shift()
var file = new_process_args.shift()

var commands = [];
try {
  commands = require(process.cwd()+"/compile_commands.json");
} catch (e) {};

commands.push({
  "file": file,
  "directory": directory,
  "command": new_process_args.join(" ")
});

var ws = fs.createWriteStream("./compile_commands.json")

ws.on("open", (fd) => {
  ws.write(JSON.stringify(commands, null, 2))
})

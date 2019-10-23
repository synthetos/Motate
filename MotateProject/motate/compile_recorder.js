#!/usr/bin/env node
let fs = require("fs")

// Copy the argv
let new_process_args = process.argv.slice(2)

let build_directory = new_process_args.shift()
let output_directory = new_process_args.shift()
let file = new_process_args.shift()

let commands = [];
let file_commands = {};
try {
  let incoming_commands = require(output_directory+"/compile_commands.json");

  // strip out commands for this file
  for (let command of incoming_commands) {
    // if (file !== command.file) {
      commands.push(command);
    // }
  }
} catch (e) {};

commands.push({
  "file": file,
  "directory": build_directory,
  "command": new_process_args.join(" ")
});

let ws = fs.createWriteStream(output_directory+"/compile_commands.json")

ws.on("open", (fd) => {
  ws.write(JSON.stringify(commands, null, 2))
})

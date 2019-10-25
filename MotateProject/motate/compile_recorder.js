#!/usr/bin/env node
let fs = require("fs")

// Copy the argv
let new_process_args = process.argv.slice(2)

let build_directory = new_process_args.shift()
let output_directory = new_process_args.shift()
let file = new_process_args.shift()

let commands = [];
let file_commands = {};
let should_write = true;
try {
  let incoming_commands = require(output_directory+"/compile_commands.json");

  for (let command of incoming_commands) {
    // check for the file already being shown
    if (file === command.file && build_directory === command.directory && new_process_args.join(" ") === command.command) {
      // this command is *exactly* already in the commands list, stop
      should_write = false;
      break;
    }
    if (file === command.file && build_directory === command.directory) {
      // Don't add it to the list, removing the old version
    } else {
      commands.push(command);
    }
  }
} catch (e) {};

if (should_write) {
  commands.push({
    "file": file,
    "directory": build_directory,
    "command": new_process_args.join(" ")
  });

  let ws = fs.createWriteStream(output_directory+"/compile_commands.json")

  ws.on("open", (fd) => {
    ws.write(JSON.stringify(commands, null, 2))
  })
}

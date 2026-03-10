const checkDir = () => {
    const fs = require("fs-extra");
    try {
        console.log(fs.readdirSync("./"));
    } catch (ex) {}

    try {
        console.log(fs.readdirSync("./resources"));
    } catch (ex) {}

    console.log(`__dirname: [${__dirname}] end__dirname`);
};

checkDir();

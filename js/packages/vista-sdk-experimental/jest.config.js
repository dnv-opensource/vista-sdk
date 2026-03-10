/** @type {import('ts-jest').JestConfigWithTsJest} */
module.exports = {
    displayName: "vista-sdk-experimental",
    preset: "ts-jest",
    testEnvironment: "node",
    transform: {
        "^.+\\.ts?$": "ts-jest",
    },
    transformIgnorePatterns: ["<rootDir>/node_modules/"],
    testTimeout: 100000,
    globalSetup: "../vista-sdk/tests/setup/globalSetup.ts",
    globalTeardown: "../vista-sdk/tests/setup/globalTeardown.ts",
    setupFilesAfterEnv: ["../vista-sdk/tests/setup/setupTests.ts"],
};

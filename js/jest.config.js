/** @type {import('ts-jest').JestConfigWithTsJest} */
module.exports = {
    preset: "ts-jest",
    testEnvironment: "node",
    transform: {
        "^.+\\.ts?$": "ts-jest",
    },
    transformIgnorePatterns: ["<rootDir>/node_modules/"],
    testTimeout: 100000,
    globalSetup: "<rootDir>/tests/setup/globalSetup.ts",
    globalTeardown: "<rootDir>/tests/setup/globalTeardown.ts",
    setupFilesAfterEnv: ["<rootDir>/tests/setup/setupTests.ts"],
};

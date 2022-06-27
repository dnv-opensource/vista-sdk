import { CodebooksDto } from "./types/CodebookDto";
import { GmodDto } from "./types/GmodDto";
import fetch from "cross-fetch";
import { VisVersion, VisVersionExtension } from "./VisVersion";

export class Client {
    private static readonly API_URL = "https://mavista.azureedge.net/vis/";

    public static async visGetGmod(version: VisVersion): Promise<GmodDto> {
        const url = `${this.API_URL}gmod-vis-${VisVersionExtension.toString(
            version
        )}.json`;
        const options: RequestInit = {
            method: "GET",
            headers: {
                "Content-Type": "application/json"
            }
        };
        const response = await fetch(url, options);
        if (response.ok) {
            return (await response.json()) as GmodDto;
        }

        throw new Error(
            `Failed to fetch gmod version ${VisVersionExtension.toString(
                version
            )}: ${response.statusText}.`
        );
    }

    public static async visGetCodebooks(
        version: VisVersion
    ): Promise<CodebooksDto> {
        const url = `${
            this.API_URL
        }codebooks-vis-${VisVersionExtension.toString(version)}.json`;
        const options: RequestInit = {
            method: "GET",
            headers: {
                "Content-Type": "application/json",
            },
        };
        const response = await fetch(url, options);
        if (response.ok) {
            return (await response.json()) as CodebooksDto;
        }

        throw new Error(
            `Failed to fetch codebook version ${VisVersionExtension.toString(
                version
            )}: ${response.statusText}.`
        );
    }
}

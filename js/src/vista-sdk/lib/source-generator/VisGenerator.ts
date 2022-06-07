export class VisGenerator {
    public static assembleVisVersionFile(versions: string[]) {
        return `
        ${this.generateVisVersionEnum(versions)}
        
        ${this.generateVisVersionExtensionClass(versions)}

        ${this.generateVisVersionsClass(versions)}
        `;
    }

    private static generateVisVersionEnum(versions: string[]) {
        return `export enum VisVersion {
            ${versions.map(
                (v) => `${this.toVersionEnumKeyString(v)} = "${v}",\n`
            )}
        }`;
    }

    private static generateVisVersionExtensionClass(versions: string[]) {
        return `export class VisVersionExtension {
            public static toVersionString(version: VisVersion, builder?: string[]): string {
              let v: string;
              switch (version) {
                  ${versions.map(
                      (v) => `case VisVersion.${this.toVersionEnumKeyString(v)}:
                  v = "${v}";
                  break;`
                  )}
                default:
                  throw new Error('Invalid VisVersion enum value: ' + version);
              }
          
              if (builder) {
                builder.push(v);
              }
              return v;
            }
          
            public static toString(version: VisVersion, builder?: string[]): string {
              let v: string;
              switch (version) {
                    ${versions.map(
                        (v) => `case VisVersion.${this.toVersionEnumKeyString(
                            v
                        )}:
                    v = "${v}";
                    break;`
                    )}
                default:
                  throw new Error('Invalid VisVersion enum value: ' + version);
              }
          
              if (builder) {
                builder.push(v);
              }
              return v;
            }
          
            public static isValid(version: VisVersion): boolean {
              switch (version) {
                    ${versions.map(
                        (v) => `case VisVersion.${this.toVersionEnumKeyString(
                            v
                        )}:
                    return true;\n`
                    )}
                default:
                  return false;
              }
            }
          }`;
    }

    private static generateVisVersionsClass(versions: string[]) {
        return `export class VisVersions {
            public static get all(): VisVersion[] {
              return Object.values(VisVersion)
                .map(v => this.tryParse(v))
                .filter(v => v) as VisVersion[];
            }
          
            public static parse(version: string): VisVersion {
              const v = this.tryParse(version);
              if (!v) {
                throw new Error('Couldnt parse version string: ' + version);
              }
          
              return v;
            }
          
            public static tryParse(version: string): VisVersion | undefined {
              switch (version) {
                  ${versions.map(
                      (v) => `case "${v}":
                    return VisVersion.${this.toVersionEnumKeyString(v)};\n`
                  )}
                default:
                  return;
              }
            }
          }`;
    }

    private static toVersionEnumKeyString(v: string) {
        return `v${v.replace("-", "_")}`;
    }
}

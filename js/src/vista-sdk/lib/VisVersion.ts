
        export enum VisVersion {
            v3_4a = "3-4a",

        }
        
        export class VisVersionExtension {
            public static toVersionString(version: VisVersion, builder?: string[]): string {
              let v: string;
              switch (version) {
                  case VisVersion.v3_4a:
                  v = "3-4a";
                  break;
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
                    case VisVersion.v3_4a:
                    v = "3-4a";
                    break;
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
                    case VisVersion.v3_4a:
                    return true;

                default:
                  return false;
              }
            }
          }

        export class VisVersions {
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
                  case "3-4a":
                    return VisVersion.v3_4a;

                default:
                  return;
              }
            }
          }
        
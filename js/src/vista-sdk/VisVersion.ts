
        export enum  VisVersion{
            v3_4a = "3-4a"
,v3_5a = "3-5a"

        }

        export class VisVersionExtension {
            public static toVersionString(version: VisVersion, builder?: string[]): string {
              let v: string;
              switch (version) {
                  case VisVersion.v3_4a:
                  v = "3-4a";
                  break;case VisVersion.v3_5a:
                  v = "3-5a";
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
                    break;case VisVersion.v3_5a:
                    v = "3-5a";
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
case VisVersion.v3_5a:
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
case "3-5a":
                    return VisVersion.v3_5a;

                default:
                  return;
              }
            }
          }
        
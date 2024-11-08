
        export enum  VisVersion{
            v3_4a = "3-4a"
,v3_5a = "3-5a"
,v3_6a = "3-6a"
,v3_7a = "3-7a"
,v3_8a = "3-8a"

        }

        export class VisVersionExtension {
            public static toVersionString(version: VisVersion, builder?: string[]): string {
              let v: string;
              switch (version) {
                  case VisVersion.v3_4a:
                  v = "3-4a";
                  break;case VisVersion.v3_5a:
                  v = "3-5a";
                  break;case VisVersion.v3_6a:
                  v = "3-6a";
                  break;case VisVersion.v3_7a:
                  v = "3-7a";
                  break;case VisVersion.v3_8a:
                  v = "3-8a";
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
                    break;case VisVersion.v3_6a:
                    v = "3-6a";
                    break;case VisVersion.v3_7a:
                    v = "3-7a";
                    break;case VisVersion.v3_8a:
                    v = "3-8a";
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
case VisVersion.v3_6a:
                    return true;
case VisVersion.v3_7a:
                    return true;
case VisVersion.v3_8a:
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
case "3-6a":
                    return VisVersion.v3_6a;
case "3-7a":
                    return VisVersion.v3_7a;
case "3-8a":
                    return VisVersion.v3_8a;

                default:
                  return;
              }
            }
          }
        
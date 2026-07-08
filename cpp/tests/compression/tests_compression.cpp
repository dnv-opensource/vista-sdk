#include <doctest/doctest.h>

#include <Compression/Gzip.h>
#include <Compression/Inflate.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace dnv::vista::sdk::tests
{
    namespace
    {
        std::vector<uint8_t> fromHex(std::string_view hex)
        {
            std::vector<uint8_t> out;
            out.reserve(hex.size() / 2);
            for (size_t i = 0; i + 1 < hex.size(); i += 2)
            {
                auto nibble = [](char c) -> uint8_t {
                    if (c >= '0' && c <= '9')
                    {
                        return static_cast<uint8_t>(c - '0');
                    }
                    if (c >= 'a' && c <= 'f')
                    {
                        return static_cast<uint8_t>(c - 'a' + 10);
                    }
                    return static_cast<uint8_t>(c - 'A' + 10);
                };
                out.push_back(static_cast<uint8_t>((nibble(hex[i]) << 4) | nibble(hex[i + 1])));
            }
            return out;
        }
    } // namespace

    TEST_CASE("gunzip - uncompressed block (type 00)")
    {
        // gzip wrapping a DEFLATE store block containing "Hello, World!"
        auto gz = fromHex("1f8b08000000000000ff010d00f2ff48656c6c6f2c20576f726c6421d0c34aec0d000000");
        auto result = compression::gunzip(gz);
        std::string_view sv{ reinterpret_cast<const char*>(result.data()), result.size() };
        REQUIRE_EQ(sv, "Hello, World!");
    }

    TEST_CASE("gunzip - fixed Huffman block (type 01) - Hello World")
    {
        // gzip (mtime=0, level=6) of "Hello, World!"
        auto gz = fromHex("1f8b08000000000000fff348cdc9c9d75108cf2fca49510400d0c34aec0d000000");
        auto result = compression::gunzip(gz);
        std::string_view sv{ reinterpret_cast<const char*>(result.data()), result.size() };
        REQUIRE_EQ(sv, "Hello, World!");
    }

    TEST_CASE("gunzip - fixed Huffman block (type 01) - JSON")
    {
        // gzip (mtime=0, level=6) of {"a": 1, "b": "test"}  (21 bytes -> 39 bytes)
        auto gz = fromHex("1f8b08000000000000ffab564a54b25230d451504a02d24a25a9c5254ab5007886fdf115000000");
        auto result = compression::gunzip(gz);
        std::string_view sv{ reinterpret_cast<const char*>(result.data()), result.size() };
        REQUIRE_EQ(sv, "{\"a\": 1, \"b\": \"test\"}");
    }

    TEST_CASE("gunzip - dynamic Huffman block (type 02) - small real resource")
    {
        // gzip -1 of a 1508-byte random JSON object (forces dynamic Huffman trees)
        // python3: gzip.compress(payload, compresslevel=1) where payload is 50-key JSON
        constexpr std::string_view kHex =
            "1f8b0800361a746a04ff25d4e792b24a1080e15b31ac8a0113a8280822e6888ba202b206c09c41c574ede7a3cfefa77aa6eaad9e79"
            "79e29e9ccb738e7abf1c4325bee8edab87b7e28f374c7a222e4fc2c1fc74ef5d665f575b1f50e32de7c26db1e960d2410e239b6898"
            "8fb45b7bd6f791b4181f321cc41c2c9f8e9d35f1447c66be1cca47784e3a771cc41dc4bbed9c55b4ef97e0fe9cd23e0c2ef3450753"
            "0e9e1f4bea5e0fe47f6a42112157af1a79fd7330ede0ef454ab20966a9b11b712374a8c4ec2fec60c6417f143be1777187f4f89dbc"
            "2818eefb3bea20e1e05d2b5e9ad1f210975cef373b4d4fd153c5c12cdc39dd67b12eae2f5cbbbaaa78a29d2a1da01d4d40a35528f4"
            "51112d1e3715c548b18bbd48ea1f60a8f4c8beaa8275ed8ee6e31843214dc5b335f2c0d0694bb982cf53208ffc3482339ab354528f"
            "8342a8f77aad288ca2f48948c41cb6c5befcf1dfaee0d0ea1209089f26ca1084364e88194e66822350886544fc57dd1da6537d7542"
            "0a8114efeab1a0504b2a89f66769be548c289381cd570fd402a0ffe7da4ca60b3c300b65901d72a385d88ad98042af9def181a989b"
            "d5bcc6e6ac578d667fe3162804bb54a45ad02d1d5791a0464ed9780b61ba8e26a157091fe9cfcae1ba705b46c544136c1485cd4842"
            "2eac58dae75ecddcfdcc4ed5823f7c16391fcc422d57237e66f1efc8788a4b7734f8c0f49fff4f865a1b23398cfbaa1d5f3b77672c"
            "b4bdec1f31988556349eee7bd3df46dffa3245dc46f91f45a1802156566a55f394d72484068770adcb4cb869a010abe18db2e90dfb"
            "83ee86ad4f6db89f457b6b508845147ccfa16baedddfdfaf95ed0c11ab710485583533f6ae4cab56bf2ed865234e0c2cfe060ab1d2"
            "875545305c95526eb8d95febf2687483278641ac99a27c5366a83433cfc74698be55a96cd2eb0c6350cbf76f370e4169705a132fbb"
            "f72730c9ebc3050cb950914adb73d9bf64625242b616cb5d0dd603835c87f75434c6b8a28c59e6cccdd513cd8d82300cbd5e25bad8"
            "a8e9a10496ef3db027890cfa0d50c8a5cb83406ed0dbd88a67cd4f5ced4a6fa07d81a197bea754aa3854bdbda67513de29397e849a"
            "18f4f2fe6e57a63774111372c6cff27552aac3cf81412f9e1b14428aa278747f8bba22579f97c92a1e088a41328c8e2b8a155d91b2"
            "5de8a6f6fb5714afa2cedd3834db78abeb1bfffbca06b78f1383da95fb2e060ac956be96269a5d647a69ba072a3aec9ca5272814c3"
            "86e9cffc5a3c681379f8d916eb4c981b834231465e1d2cbee9fdab6754b2cd36b99dcd8142b0a57b4e84e8775f5dd20f45b9bcd4b1"
            "e20997c12119253c98aab9e9fefb26162dd168b46da958058664857cec4dd462da234390df09f7de2e762550481621caf94ebae27e"
            "6a71cee737ca76fcac824232d4bd2e29ca64d4ccf80b932822a8c1f5e90e0cc1fc18b6aa8498d872b0df1da9393f1e55b69ecf7fe1"
            "49c50fe4050000";
        auto gz = fromHex(kHex);
        auto result = compression::gunzip(gz);
        // The decompressed payload is 1508 bytes of JSON
        REQUIRE_EQ(result.size(), size_t{ 1508 });
        REQUIRE_EQ(result[0], uint8_t{ '{' });
        REQUIRE_EQ(result[result.size() - 1], uint8_t{ '}' });
    }

    TEST_CASE("gunzip - dynamic Huffman block (type 02) - real embedded resource (small)")
    {
        // resources/gmod-vis-versioning-3-10a.json.gz (163 bytes -> 228 bytes, btype=2)
        constexpr std::string_view kHex =
            "1f8b080000000000000a458d310bc3201085f740fec371731b623b045c3b75edd0a574b07a2442a2414d97e07f2fda88d3"
            "f1beeff16e6f1b00f472a2453cc9796d0d7240d6f578caeab3e959dd1572607ff0d5fe4133094fa9783db35e1c551d68f1"
            "c8614f0900d96560dda02b0140bb9213415b938aaf8201504ec28c74b38ab0d0779ecdd6dbcdc9fcb08c5617841b2954c7"
            "8e81984e6c9bf803b922f5b1e4000000";
        auto gz = fromHex(kHex);
        auto result = compression::gunzip(gz);
        REQUIRE_EQ(result.size(), size_t{ 228 });
        REQUIRE_EQ(result[0], uint8_t{ '{' });
    }

    TEST_CASE("gunzip - dynamic Huffman block (type 02) - codebooks resource")
    {
        // resources/codebooks-vis-3-4a.json.gz (3803 bytes -> 19080 bytes, btype=2)
        // This is the resource that crashes the codebooks sample.
        constexpr std::string_view kHex =
            "1f8b080000000000000aa55b5bafe3b6117e2fd0ff60ec530a64b54ddba7a02890e6d206d866835cda87a2286889b68895448592ec"
            "6"
            "304f9effd86946c1fce0c7dba79f239f30d8714399c1bc99f7ffb9bddeed554b7b637ffb461727e78f5e9eed527d5ef5f7d1ca1fde"
            "2bae6eb06c44f12e1e4a6ef6c67cd6489f18fafff64564e37db7e02eddff4df6ef773fa0130983ef2d6a6ab97cecce8634a6d227c3"
            "2dd62a9e1b505885f7cf9d5673fbefde1bf7ffbeedd8fdfde84aea8a9eba52751b6b9098ac8d607077c08b696e8cb3067b4c69e5c1"
            "c6646b7d3ec7ac8cec80717a65c446718a9374fae5f7a995a993af869aa263b4c3e4c399335f958885469038de0438919c7ff23add"
            "24694b3c9df2c71ba41e3c4a4dbea6cddb1e5cbd7dbc6092321e2a309883c45d9f29012f58174dfe45a12cc7014696549a1cf29933"
            "b0eeee06ac3f4769acdd098d0a82bc919ca9d0bfca5093b998039adf3afdcc8e5beae5ccf7bb8f1fc67fbf397f4c72f1fab86c6f73"
            "d06fed0c87c56cfee44fb99db9715b8ebfd6eb4af3e9b661b06de2c9195469fb7b4da3bd3478b93b76d6c1da2517d3e2b6e58c965a"
            "1fe6403135947ac8a98d6dc0f03ac226fbad233bbe8a60d50e47d39987d67df80917e99581b612e355215916f7dfd9e09ea88f85cc"
            "c3244a222e5dd68f97a792266eea0f3fa64bfc34c06879dcd046d80d2f0e6309fb70b2b5d6df6139496af4e58e96ab3c94a8d88aa3"
            "559046d5e665fc102845c0d56c08fb97bec6d38daa1be482036e4623a515eb0bd9f6d1962f202469cf36215d80692a4461af53734d"
            "5fea280ac47857493a3ccedf75864618b4d89ac356a9779d7f8335f9609481511ad69e7cf4a5320e5a697a16e831f9ca0ab69d5ef1"
            "8b2a9b84314e93f043732b13311b5064b18767ed8bdd9f9c381b7045a311797a860ff30c731cc76987f6d74daef1da454c6857ce73"
            "0cade7530d167f81a8678d7d18754473309900d4a2b3f493def176869be66b509472fd12a4c44032fcd63dd049fcce897bc8bdaff8"
            "11953846f6e3856066621df8470d2fb654a31081bee1dc83f5f1f1c962ff84e94e73b1e2f119586274de48609b2c2d2c0abba2ea75"
            "fd0a0c1ba94a1aa456453ed61f687a5dff37eef5969bfaa9c0505691c8c7d270c04be76765d671bf19b9be086f7da84d819e1443ee"
            "391e8f35eec536bb07ac2c00e9d4168066f2f61f0506d555a93c421220b3e5710b988b340e48c7434610f2dcda9dda5665f770cf62"
            "20ea2b5e674a9943ee3aa23431e6d30b051b6d23fa6bd34c163a3ed99718bc8d8fa900fb4f5b32aca2c9dab8511b9c1c2d1f14973c"
            "33487a5972d58476179a5ea57829529e886634e817e4f4b37b64b501bddb160ac0f388a4b70cfd823db18a2162a8c2f5baa6e649fb"
            "4ec0372b39974984b5ebb55e7af38ac5edc8289cab47470308530bb19994b252bd2c32c70f2a5d2bd937fba70e1a39fe3de961a8c5"
            "05c3ef8600fc11d31cf3c97adcdc9c6ec98e9e0644dd14c102ed3c97344ed604838b95aeeebcccd420c1165e730754b23f05b93971"
            "1665bb70314a52b68d7dcdad083832fdaec9fb0a7f9de5dba3998ea05dbea4cb9aa48e7c388b4a4231f1254cd97d1feda88aa7e3ff"
            "833dcd611390653153398ce1f175bb9615c54d02f3347b78841041b777433a65f12bb6162c383b5cd1e63668695b3ae6991d262450"
            "b0d91d355a3776c4e068f3a15f038d9f9de3d1c28159488da40c4cfc4e6b501f91edfd4293dcd77923cd22bbd12c785bd362fb99ac"
            "ff6037571f4937b51fdf97b64f3bb8fa6d68dbf63da387a31a3dc7b54cab45406b5f599763b13b68c7033f9dabaa66165126c6abd8"
            "ef3032ce894d2dfbc0387820d0b32b189580c8135d63b78eb0728fcd29052b12e0e3e9ce9dbb39d77d0b3db38bb70d6339f5d045c4"
            "d355131858738915c94891016d3c0a5129d2cb61df2610ec831eed1a2f835b2e5f2b79057183854dbbcb7c5b17f3d74767e8305c00"
            "f2f2e25722e94688a384470e88ce4c54e0581e2304b23fc266520b9a83fa7cce42f5ab36f5b642fd3eea333721a4cf0246c26e2a89"
            "63c5a48d4ea24d2156a7516e90a157d2a837e477e77f71114e3f531f865b40d1f36363c77f3b1aefd4176e9a70511102cd3af7694f"
            "bc977d8c755bbc088b8392f6e5d713a86c376b62abeee77c650d7a85b05e970c1d4628f544ab7b0f064bd3288cda0e9e4510d47660"
            "e41c3395fa84a035a590ed121a12ea9e0ae1f978e15b436b4f71472b063a00d3e59949d85cf1e4703df38cb1f8e927e9ecc3bb0dae"
            "07c2300f8ba7c747b8868e636b7eb66c42aa0d2c37c321df0c66813313a86e560c5f348a81f3bfb248e973004a993433ec03ff58ac"
            "290e2dc6be998bba6b07ba9c5612504d657100bbb4939ea5421170e4e38b99ddcb04cd5d83a862cf084f1e0a3a21dcc6c3e6740427"
            "81"
            "c2c5cb4cc190bde964fccb247186e9f90d9c4cfafbd3de0648eaa7f8c339c623299c7916636d5097bae6708ac099f95958a5889725"
            "d"
            "c46b14eb0bfad1d8916947a455f8587cf57bcb92a0c69ed740ec2ef765612f0c9650387287180a925e555119a4a6773ca51ea6b133"
            "b"
            "514e753f14ad0a18d5c58ef2bcb8c12f582428c58fd0282637b84d4a642f6c9cfa95038a7b3c99c8c020b8a1e3975114d527381c58"
            "7"
            "cae01205a92f5bde4d7d98adb274a10010965ede63f1340195870aa77e388551d105d720d8cc5ee183b3147d21381c8e4c7fae5c08"
            "1"
            "5d67b1c1a876665ee18b89949a0ef2ef8d0aa60192c8cc291135106e946818cc2434e3dc02bc15367444c712b6528086f99d2693e2"
            "5"
            "a6db54c07dca8023b4c46a7b03233f549d3b303a8c3cc5a58a71da6094eb8496f967447bb17a887cd8455b4245355b72b42b031c88"
            "e"
            "4bd132a3ab7965ba1361efc3322aa0f23a2323570b9e7504217aac0c240d0a7d2a44999310260682d3641ae1b08116c23d81e39747"
            "0"
            "9abb43711df7ace68201200ec2451da77b4d5ce59419c1d9cb7b1e6175d2c6ee7072ca2bb9834578237ba1157c89e35b596152e0cc"
            "6"
            "5ece4f649ad5415628cdaf6eb782113e119391c6eb8353dd9e8b17e328d2888143c8bc05cf23131892faa606f2676990824b287316"
            "b"
            "172f681183b67b1318280e971b62678ae6b8478512c777296814e018178b44162c0f3ed631c9afb0af13cfb6e86606cd90f03d88ab"
            "4"
            "b61468471f0589cb0d1201012b70226835af18d730328f64aeb08fdc77511d2c7cef52e9f0bd1e2a5e4738b85988118dd5cb72553b"
            "b"
            "3228fb24a172c7285a5531dd14e8f1ee0cbfb2b655cb18996b43a455a25f55162ac07721e95d2d1dd497f6b3c4c4429d44a465d0a2"
            "ea1b075d07602055be35c8e507e248b4445b1ee95a6a8b8b3d2682b2fdbcc24a9a7ec58bb2b7892d58df2bef83d14e72689d002906"
            "c3651b2a1470e540fcd6951438e18afe465054df80a3259296ce4b44db8f1b41d16608a67e32f020fc23e241520a88b125e97a30ed"
            "e"
            "5d354c1d930cba0329fb00ac289d28d3bb9851788bd32aa22a91a6ec5ece47ae85549a9df04971394cc2161a283be42dc6cd32d54f"
            "2"
            "abbe1a3ceefc54d2d6412dc9359554bd9962ce2d0639f790b27d569627d827bef79f839514db24166927dc21a596b245c3f2d57431"
            "5"
            "7f9ae1b2a662af7b890b25ce152181f1548a2152298c7414b523a32b7526a49282574ca9c2c6851b2072b836210122a5a84e96c2e4"
            "5"
            "c111d7e412a888b5b0418fad016793376de42bf424ed65bd3a32dba7593924d68b6c198764ce379694d9eb22544f1119fc56de287c"
            "4"
            "0eb7096cbcd09f40fc72739915bf278397c7b87d14820ae8cc290c111c8e04f0bd64c6c180b63d08b1cf301ee861329eea303d687c"
            "9"
            "8bc4abf93018e16142682805bef3f58cef6156c2584bdb1bf518763961097b292e59ba8e47c6cb5c572863a68bfb0226904f166ad0"
            "5"
            "9adb2abd4d4d3064b7dd1955b49b0538ad4885684a05e2cbc26a29a4f6db09c51ada8e84defdc73b5c7279ea52aec998a250fcd0fe"
            "7"
            "12ad4f64132b2a111133a0888806eb8c91309224dbf193093a007cc167312ef9b3884daaca45a0186e3de710a55fccb9e45722ac6c"
            "7"
            "5c2d6a9fb9053c0e8685ff06ae57a19a629dd94518fb2e3b317e5d10b2f6589e448a462741cb2da11a6909f671b1c9bb1d334a2557"
            "b"
            "7ac1c1ac41c2824540279c01f53d995e41641d90bb52f1420211adc01606c14d1022b5d8b8b54f3d19d7896f5ecc15c94ce3a0212b"
            "5"
            "d32632932bd355446f3d77522f3b16c13ac35bbbc1e5111921a225a4a88d2f4738a09f8cd903a92a18e85f74be148f32a3c5e4a007"
            "f"
            "82b421ba48dc1b6c1e3db46a949756cd23b985b756cd4b1e5b09d3bbbeaa6263ba3db82a0c2b5d61dbc52787c23ba914f8118bfcb6"
            "4"
            "606e36b0f195a6f9ec9e076a18e3a168ce706d3a555af7f52ca87778b709d8926583ec48af1db062992bf58b0a598485c0022bad2e"
            "6"
            "cbed5913bff473454a0fa17072507e2ed728efe40acd8e8865a46689ae34fb0a56832272767d6ba5674be53099b87ca0a0c9f0eae0"
            "4"
            "295e314eb2a3c49136434ba6e057343e503963a574b6c5e30c77beb72c3547b55b0b57e2961e95e8f06c6e7621ae6470dba3d97ba7"
            "2"
            "68cb866c068518bedc870d501b06fb86ee08f3866c3cc4a689e9527892c9202a5d7fd59bf9333774b88b4b1afabc730ab533126d73"
            "2"
            "2730f30f8189a6bddfe3d9dadbc592f880ba636d19f0b5d4f6464ea76d95cbfbdd722e990ecbabb224ad3b738a49346498777d2a7c"
            "f"
            "958cac877de570ec2d1d52ecc88f4b5dd2f1a8f46645a09f1dd2f8f5d063123068850c46621c24ab427215a004a9b0d06f7126b8a3"
            "6b50c2eb607a439120adb1378db05ea4755d82d48662602506aeb7c737ea7365d68688a472c585fc879397b05fe962a576031599df"
            "c"
            "4233a26329ddce5c1c15e24272205ad4a1fefb4a79c2c4e20a68cc25e753e938cf342693d135978e29d13136779dad1c99a11f27d5"
            "e"
            "4180982ed29e2dc0577a7a1b5de89c0c627cba276aed0a151a075c2a97dec0df20a5f177b865c2631fba7f7811be55a213cdc5a2df"
            "8"
            "7bd8f6f8a2fdd25c31b1fbb17ecaef4de3d79f20729121ac6349075b8d285e08057e4d7a0503ebe2a6a417a4b2e7c707a4dae8f9bd"
            "e"
            "9303d69f94037cf8aa5c6d1fb172737a24ff7acff5e8e1ebf95ba0d43c7c93ce66fa1e2abc4aa7d5901ea61742f71fa5f12061d15f"
            "8"
            "0c034f216a8f0809a07b889aa098a45abdd9de59073bfb55c7a6f61229f22f65f06f51241b3cf2b5d6b86e93deea86cce1a12822ac"
            "b"
            "b3a5ddfedc6a61f403e22fff035711ee15884a0000";
        auto gz = fromHex(kHex);
        auto result = compression::gunzip(gz);
        REQUIRE_EQ(result.size(), size_t{ 19080 });
        REQUIRE_EQ(result[0], uint8_t{ '{' });
    }

    TEST_CASE("gunzip - truncated FEXTRA throws")
    {
        // gzip header with FEXTRA set (FLG bit 2), xlen=0x7FFF but only 2 extra bytes present
        // Header: ID1 ID2 CM FLG MTIME(4) XFL OS xlen_lo xlen_hi extra_byte extra_byte
        // FLG=0x04 means FEXTRA is set
        auto gz = fromHex(
            "1f8b"     // ID1 ID2
            "08"       // CM = deflate
            "04"       // FLG = FEXTRA
            "00000000" // MTIME
            "00"       // XFL
            "ff"       // OS
            "ff7f"     // xlen = 0x7FFF (little-endian), claims 32767 extra bytes
            "aabb"     // only 2 bytes of extra data (truncated)
        );
        CHECK_THROWS_AS((void)compression::gunzip(gz), std::runtime_error);
    }

    TEST_CASE("gunzip - truncated dynamic Huffman block throws instead of looping forever")
    {
        auto full = fromHex("1f8b08000000000000ff05e0019624499224597e406251f3c8ead9fb1f771ff8ff031df722c605000000");
        REQUIRE_EQ(full.size(), size_t{ 42 });

        // Full stream still decompresses correctly.
        auto result = compression::gunzip(full);
        REQUIRE_EQ(result, std::vector<uint8_t>(5, 0));

        std::vector<uint8_t> truncated(full.begin(), full.begin() + 28);
        CHECK_THROWS_AS((void)compression::gunzip(truncated), std::runtime_error);
    }
} // namespace dnv::vista::sdk::tests

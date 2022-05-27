using Microsoft.Extensions.DependencyInjection;

using Vista.SDK;

namespace Vista.SDK.Tests
{
    public class CodebooksTests
    {
        public static (IServiceProvider ServiceProvider, IVIS Vis) GetVis()
        {
            var services = new ServiceCollection();
            services.AddVIS();
            var sp = services.BuildServiceProvider();

            var vis = sp.GetRequiredService<IVIS>();
            return (sp, vis);
        }

        [Fact]
        public void Test_Codebooks_Loads()
        {
            var (_, vis) = GetVis();

            var codebooks = vis.GetCodebooks(VisVersion.v3_4a);
            Assert.NotNull(codebooks);

            Assert.NotNull(codebooks.GetCodebook(CodebookName.Position));
        }

        [Fact]
        public void Test_Codebooks_Equality()
        {
            var (_, vis) = GetVis();

            var codebooks = vis.GetCodebooks(VisVersion.v3_4a);

            Assert.True(codebooks[CodebookName.Position].HasStandardValue("centre"));
        }
    }
}

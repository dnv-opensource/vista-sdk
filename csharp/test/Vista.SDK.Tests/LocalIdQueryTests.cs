using Vista.SDK.Transport.Json;

namespace Vista.SDK.Tests;

public class LocalIdQueryTests
{
    public sealed record InputData(string LocalId, LocalIdQuery Query, bool ExpectedMatch);

    public static IEnumerable<object[]> Test_Data =>

        [
            [
                new InputData(
                    "/dnv-v2/vis-3-4a/1021.1i-6P/H123/meta/qty-volume/cnt-cargo/pos~percentage",
                    LocalIdQueryBuilder.Empty().WithPrimaryItem(GmodPathQueryBuilder.From(GmodPath.Parse("1021.1i-6P/H123", VisVersion.v3_4a)).WithoutLocations().Build()).Build(),
                    true
                )
            ],
            [
                new InputData(
                    "/dnv-v2/vis-3-4a/652.31/S90.3/S61/sec/652.1i-1P/meta/cnt-sea.water/state-opened",
                    LocalIdQueryBuilder.Empty().WithTag(CodebookName.Content, "sea.water").Build(),
                    true
                )
            ],
            [
                new InputData(
                    "/dnv-v2/vis-3-4a/411.1/C101.31-2/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
                    LocalIdQueryBuilder.Empty().WithPrimaryItem(GmodPath.Parse("411.1/C101.31-1", VisVersion.v3_4a)).Build(),
                    false
                )
            ],
            [
                new InputData(
                    "/dnv-v2/vis-3-4a/411.1-1/C101.63/S206/~propulsion.engine/~cooling.system/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
                    LocalIdQueryBuilder.Empty().WithPrimaryItem(GmodPathQueryBuilder.From(GmodPath.Parse("411.1-2/C101.63/S206", VisVersion.v3_4a)).WithoutLocations().Build()).Build(),
                    true
                )
            ],
            [
                new InputData(
                    "/dnv-v2/vis-3-4a/411.1/C101.63/S206/sec/411.1/C101.31-5/~propulsion.engine/~cooling.system/~for.propulsion.engine/~cylinder.5/meta/qty-temperature/cnt-exhaust.gas/pos-inlet",
                    LocalIdQueryBuilder.Empty().WithSecondaryItem(GmodPathQueryBuilder.From(GmodPath.Parse("/411.1/C101.31-2", VisVersion.v3_4a)).WithoutLocations().Build()).Build(),
                    true
                )
            ],
            [
                new InputData(
                    "/dnv-v2/vis-3-4a/511.11-21O/C101.67/S208/meta/qty-pressure/cnt-air/state-low",
                    LocalIdQueryBuilder.Empty().WithPrimaryItem(GmodPathQueryBuilder.From(GmodPath.Parse("411.1", VisVersion.v3_4a)).WithoutLocations().Build()).Build(),
                    false
                )
            ]
        ];

    [Theory]
    [MemberData(nameof(Test_Data))]
    public void Test_Matches(InputData data)
    {
        var localId = LocalId.Parse(data.LocalId);
        Assert.NotNull(localId);

        var match = data.Query.Match(localId);
        Assert.Equal(data.ExpectedMatch, match);
    }

    [Fact]
    public void Test_Happy_Path()
    {
        var localId = LocalId.Parse(
            "/dnv-v2/vis-3-4a/1036.13i-1/C662/sec/411.1-2/C101/meta/qty-pressure/cnt-cargo/state-high.high/pos-stage-3/detail-discharge"
        );

        var primaryItem = localId.PrimaryItem;
        var secondaryItem = localId.SecondaryItem!;

        // Match exact
        LocalIdQueryBuilder builder = LocalIdQueryBuilder.From(localId);
        LocalIdQuery query = builder.Build();
        Assert.True(query.Match(localId));

        MatchCombination(individualized: true);
        MatchCombination(individualized: false);

        void MatchCombination(bool individualized)
        {
            var primaryQueryBuilder = GmodPathQueryBuilder.From(primaryItem);
            var secondaryQueryBuilder = GmodPathQueryBuilder.From(secondaryItem);

            if (individualized == false)
            {
                primaryQueryBuilder = primaryQueryBuilder.WithoutLocations();
                secondaryQueryBuilder = secondaryQueryBuilder.WithoutLocations();
            }

            var primaryQuery = primaryQueryBuilder.Build();
            var secondaryQuery = secondaryQueryBuilder.Build();

            // Match primary
            builder = LocalIdQueryBuilder.Empty().WithPrimaryItem(primaryQuery);
            query = builder.Build();
            Assert.True(query.Match(localId));

            // Match secondary
            builder = LocalIdQueryBuilder.Empty().WithSecondaryItem(secondaryQuery);
            query = builder.Build();
            Assert.True(query.Match(localId));

            // Match tags
            builder = LocalIdQueryBuilder.Empty();
            foreach (var tag in localId.MetadataTags)
                builder = builder.WithTag(tag.Name, tag.Value);
            query = builder.Build();
            Assert.True(query.Match(localId));

            // Match primary and secondary
            builder = LocalIdQueryBuilder.Empty().WithPrimaryItem(primaryQuery).WithSecondaryItem(secondaryQuery);
            query = builder.Build();
            Assert.True(query.Match(localId));

            // Match primary and tags
            builder = LocalIdQueryBuilder.Empty().WithPrimaryItem(primaryQuery);
            foreach (var tag in localId.MetadataTags)
                builder = builder.WithTag(tag.Name, tag.Value);
            query = builder.Build();
            Assert.True(query.Match(localId));

            // Match secondary and tags
            builder = LocalIdQueryBuilder.Empty().WithSecondaryItem(secondaryQuery);
            foreach (var tag in localId.MetadataTags)
                builder = builder.WithTag(tag.Name, tag.Value);
            query = builder.Build();
            Assert.True(query.Match(localId));

            // Match primary, secondary, and tags
            builder = LocalIdQueryBuilder.Empty().WithPrimaryItem(primaryQuery).WithSecondaryItem(secondaryQuery);
            foreach (var tag in localId.MetadataTags)
                builder = builder.WithTag(tag.Name, tag.Value);
            query = builder.Build();
            Assert.True(query.Match(localId));
        }
    }

    [Fact]
    public void Test_Variations()
    {
        var localId = LocalId.Parse(
            "/dnv-v2/vis-3-4a/1036.13i-1/C662/sec/411.1-2/C101/meta/qty-pressure/cnt-cargo/state-high.high/pos-stage-3/detail-discharge"
        );

        var primaryItem = GmodPath.Parse("1036.13i-2/C662", VisVersion.v3_4a);

        var builder = LocalIdQueryBuilder.Empty().WithPrimaryItem(primaryItem);
        var query = builder.Build();
        Assert.False(query.Match(localId));

        builder = builder.WithPrimaryItem(primaryItem, builder => builder.WithoutLocations().Build());
        query = builder.Build();
        Assert.True(query.Match(localId));

        primaryItem = primaryItem.WithoutLocations();
        builder = LocalIdQueryBuilder.Empty().WithPrimaryItem(primaryItem);
        query = builder.Build();
        Assert.False(query.Match(localId));

        builder = builder.WithPrimaryItem(primaryItem, builder => builder.WithoutLocations().Build());
        query = builder.Build();
        Assert.True(query.Match(localId));
    }

    [Theory]
    [InlineData("/dnv-v2/vis-3-4a/623.121/H201/sec/412.722-F/C542/meta/qty-level/cnt-lubricating.oil/state-low")]
    [InlineData("/dnv-v2/vis-3-4a/412.723-F/C261/meta/qty-temperature/state-high")]
    [InlineData("/dnv-v2/vis-3-4a/412.723-A/C261/meta/qty-temperature/state-high")]
    [InlineData("/dnv-v2/vis-3-4a/412.723-A/C261/sec/411.1/C101/meta/qty-temperature/state-high/cmd-slow.down")]
    [InlineData("/dnv-v2/vis-3-4a/623.1/sec/412.722-F/CS5/meta/qty-level/cnt-lubricating.oil/state-high")]
    [InlineData("/dnv-v2/vis-3-4a/623.1/sec/412.722-F/CS5/meta/qty-level/cnt-lubricating.oil/state-low")]
    [InlineData("/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/state-running")]
    [InlineData("/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/state-failure")]
    [InlineData("/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/cmd-start")]
    [InlineData("/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/cmd-stop")]
    [InlineData(
        "/dnv-v2/vis-3-4a/623.22i-1/S110.2/E31/sec/412.722-F/C542/meta/qty-electric.current/cnt-lubricating.oil"
    )]
    [InlineData("/dnv-v2/vis-3-4a/623.22i-1/S110/sec/412.722-F/C542/meta/state-remote.control")]
    [InlineData("/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/state-running")]
    [InlineData("/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/state-failure")]
    [InlineData("/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/cmd-start")]
    [InlineData("/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/cmd-stop")]
    [InlineData(
        "/dnv-v2/vis-3-4a/623.22i-2/S110.2/E31/sec/412.722-F/C542/meta/qty-electric.current/cnt-lubricating.oil"
    )]
    [InlineData("/dnv-v2/vis-3-4a/623.22i-2/S110/sec/412.722-F/C542/meta/state-remote.control")]
    [InlineData("/dnv-v2/vis-3-4a/623.22i/S110/sec/412.722-F/C542/meta/state-stand.by/cmd-start")]
    [InlineData("/dnv-v2/vis-3-4a/623.1/sec/412.722-F/C542/meta/qty-level/cnt-lubricating.oil/state-low")]
    [InlineData("/dnv-v2/vis-3-4a/623.22i/S110/sec/412.722-F/C542/meta/state-control.location")]
    [InlineData("/dnv-v2/vis-3-4a/623.22i/S110/sec/412.722-F/C542/meta/detail-stand.by.start.or.power.failure")]
    [InlineData("/dnv-v2/vis-3-4a/623.1/sec/412.722-F/C542/meta/qty-level/cnt-lubricating.oil/state-high")]
    [InlineData("/dnv-v2/vis-3-4a/412.723-F/C261/meta/qty-temperature")]
    [InlineData("/dnv-v2/vis-3-4a/412.723-A/C261/meta/qty-temperature")]
    [InlineData("/dnv-v2/vis-3-4a/623.121/H201/sec/412.722-A/C542/meta/qty-level/cnt-lubricating.oil/state-high")]
    [InlineData("/dnv-v2/vis-3-4a/623.121/H201/sec/412.722-A/C542/meta/qty-level/cnt-lubricating.oil/state-low")]
    [InlineData("/dnv-v2/vis-3-4a/412.723-A/CS6d/meta/qty-temperature")]
    public void Test_Samples(string localIdStr)
    {
        var localId = LocalId.Parse(localIdStr);
        Assert.NotNull(localId);

        var builder = LocalIdQueryBuilder.From(localId);
        var query = builder.Build();
        Assert.True(query.Match(localId));

        query = LocalIdQueryBuilder.Empty().Build();
        Assert.True(query.Match(localId));
    }

    [Fact]
    public async void Test_Consistency_Smoke_Test()
    {
        await using (var file = File.OpenRead("testdata/LocalIds.txt"))
        {
            using var reader = new StreamReader(file, leaveOpen: true);

            var errored = new List<(string Path, Exception? exception)>();

            string? localIdStr;
            while ((localIdStr = await reader.ReadLineAsync()) is not null)
            {
                try
                {
                    var localId = LocalId.Parse(localIdStr);
                    var builder = LocalIdQueryBuilder.From(localId);
                    Assert.NotNull(builder);
                    var query = builder.Build();
                    Assert.NotNull(query);
                    var match = query.Match(localId);
                    if (match == false)
                        errored.Add((localIdStr, null));
                }
                catch (Exception e)
                {
                    errored.Add((localIdStr, e));
                }
            }
            if (errored.Count > 0)
            {
                foreach (var (path, exception) in errored)
                {
                    Console.WriteLine($"Failed on {path}");
                    if (exception != null)
                        Console.WriteLine(exception);
                }
            }
            Assert.Empty(errored);
        }
    }

    [Theory]
    [InlineData("schemas/json/DataChannelList.sample.json", 3, 8, 3, 1, 4)]
    public async void Test_DataChannelList_Filter(string file, params int[] queryMatches)
    {
        var gmod = VIS.Instance.GetGmod(VisVersion.v3_4a);
        var locations = VIS.Instance.GetLocations(VisVersion.v3_4a);
        var codebooks = VIS.Instance.GetCodebooks(VisVersion.v3_4a);

        var pPath = gmod.ParsePath("621.11i/H135");
        var sPath = gmod.ParsePath("1036.13i-1/C662.1/C661");
        var tag = codebooks.CreateTag(CodebookName.Content, "heavy.fuel.oil");
        var location = locations.Parse("P");

        var localId = LocalId.Parse(
            "/dnv-v2/vis-3-4a/1036.11/S90.3/S61/sec/1036.13i-1/C662.1/C661/meta/state-auto.control/detail-blow.off"
        );

        await using var reader = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.Read);

        var package = await Serializer.DeserializeDataChannelListAsync(reader);
        Assert.NotNull(package);

        List<LocalIdQuery> queries = new(queryMatches.Length);

        var query1 = LocalIdQueryBuilder.Empty().WithTag(tag).Build();
        var query2 = LocalIdQueryBuilder
            .Empty()
            .WithPrimaryItem(pPath, builder => builder.WithoutLocations().Build())
            .Build();
        var query3 = LocalIdQueryBuilder
            .Empty()
            .WithPrimaryItem(pPath, builder => builder.WithNode(nodes => nodes["621.11i"], [location]).Build())
            .Build();
        var query4 = LocalIdQueryBuilder.Empty().WithSecondaryItem(sPath).Build();
        var query5 = LocalIdQueryBuilder
            .Empty()
            .WithSecondaryItem(sPath, builder => builder.WithNode(nodes => nodes["1036.13i"], true).Build())
            .Build();

        queries.Add(query1);
        queries.Add(query2);
        queries.Add(query3);
        queries.Add(query4);
        queries.Add(query5);
        Assert.Equal(queryMatches.Length, queries.Count);

        for (var i = 0; i < queries.Count; i++)
        {
            var query = queries[i];
            var matches = 0;
            foreach (var channel in package.Package.DataChannelList.DataChannel)
            {
                if (query.Match(channel.DataChannelID.LocalID))
                    matches++;
            }

            Assert.Equal(matches, queryMatches[i]);
        }
    }

    [Fact]
    public void Test_Use_Case_1()
    {
        var localId = LocalId.Parse(
            "/dnv-v2/vis-3-7a/433.1-P/C322/meta/qty-linear.vibration.amplitude/pos-driving.end/detail-iso.10816"
        );

        var query = LocalIdQueryBuilder
            .From(localId)
            .WithPrimaryItem(localId.PrimaryItem, builder => builder.WithNode(nodes => nodes["433.1"], true).Build())
            .Build();
        Assert.True(query.Match(localId));
        Assert.True(
            query.Match(
                "/dnv-v2/vis-3-7a/433.1-S/C322/meta/qty-linear.vibration.amplitude/pos-driving.end/detail-iso.10816"
            )
        );
    }

    [Fact]
    public void Test_Use_Case_2()
    {
        var localId = LocalId.Parse(
            "/dnv-v2/vis-3-7a/511.31/C121/meta/qty-linear.vibration.amplitude/pos-driving.end/detail-iso.10816"
        );
        var gmod = VIS.Instance.GetGmod(localId.VisVersion);
        // Match all Wind turbine arrangements
        var query = LocalIdQueryBuilder
            .From(localId)
            .WithPrimaryItem(builder => builder.WithNode(gmod["511.3"], true).Build())
            .Build();
        Assert.True(query.Match(localId));

        // Should not match Solar pannel arrangements
        query = LocalIdQueryBuilder
            .Empty()
            .WithPrimaryItem(builder => builder.WithNode(gmod["511.4"], true).Build())
            .Build();

        Assert.False(query.Match(localId));
    }
}

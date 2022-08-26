using System.Text;
using System.Threading.Channels;

namespace Vista.SDK.SmokeTests;

public class GmodVersioningTests
{
    private async Task VersioningTest()
    {
        var sourceGmod = VIS.Instance.GetGmod(VisVersion.v3_4a);
        var targetGmod = VIS.Instance.GetGmod(VisVersion.v3_5a);

        var channel = Channel.CreateUnbounded<GmodPath>(
            new UnboundedChannelOptions
            {
                SingleReader = false,
                SingleWriter = true,
                AllowSynchronousContinuations = true,
            }
        );
        var context = new SmokeTestContext(targetGmod, channel);
        const int pad = 7;

        var producer = Task.Run(
            () =>
            {
                var completed = sourceGmod.Traverse(
                    context,
                    (context, parents, node) =>
                    {
                        if (parents.Count == 0)
                            return TraversalHandlerResult.Continue;

                        var path = new GmodPath(parents.ToArray(), node, skipVerify: true);
                        _ = context.Channel.Writer.TryWrite(path);
                        return TraversalHandlerResult.Continue;
                    }
                );
                Assert.True(completed);
                context.Channel.Writer.Complete();
                var counter = Interlocked.Read(ref context.Counter);
                Console.WriteLine($"[{counter, pad}] Done traversing {sourceGmod.VisVersion} gmod");
            }
        );

        var consumers = new Task[Environment.ProcessorCount];
        for (int i = 0; i < consumers.Length; i++)
        {
            var capturedI = i;
            consumers[i] = Task.Run(
                async () =>
                {
                    var thread = capturedI;
                    var reader = context.Channel.Reader;

                    while (await reader.WaitToReadAsync())
                    {
                        while (reader.TryRead(out var sourcePath))
                        {
                            GmodPath? targetPath = null;
                            long counter = 0;
                            try
                            {
                                targetPath = VIS.Instance.ConvertPath(
                                    VisVersion.v3_4a,
                                    sourcePath,
                                    VisVersion.v3_5a
                                );
                                Assert.NotNull(targetPath);
                                var parsedPath = context.TargetGmod.TryParsePath(
                                    targetPath!.ToString(),
                                    out var parsedTargetPath
                                );
                                Assert.True(parsedPath);
                                Assert.Equal(parsedTargetPath?.ToString(), targetPath.ToString());
                                counter = Interlocked.Increment(ref context.Counter);

                                var sourcePathStr = sourcePath.ToString();
                                var targetPathStr = targetPath.ToString();
                                if (!sourcePathStr.SequenceEqual(targetPathStr))
                                {
                                    lock (context.ChangedPaths)
                                    {
                                        context.ChangedPaths.Add((sourcePathStr, targetPathStr));
                                    }
                                }
                            }
                            catch (Exception e)
                            {
                                counter = Interlocked.Increment(ref context.Counter);
                                Console.WriteLine(
                                    $"[{counter, pad}][{thread, 2}] Failed to create valid path from: {sourcePath} -> {targetPath?.ToString() ?? "N/A"} - {e.Message}"
                                );
                                lock (context.FailedConversions)
                                {
                                    context.FailedConversions.Add((sourcePath, targetPath, e));
                                }
                            }
                            if (counter % 10_000 == 0)
                            {
                                Console.WriteLine($"[{counter, pad}][{thread, 2}] Paths processed");
                            }
                        }
                    }
                }
            );
        }

        await producer;
        await Task.WhenAll(consumers);

        Console.WriteLine(
            $"[{context.Counter, pad}] Done converting from {sourceGmod.VisVersion} to {targetGmod.VisVersion}"
        );

        var success = context.Counter - context.FailedConversions.Count;
        var failed = context.FailedConversions.Count;
        var successRate = (double)success / (double)context.Counter * 100.0d;

        Console.WriteLine($"Success/failed - {success}/{failed} ({successRate:0.00}% success)");
        var distinctExceptions = context.FailedConversions
            .GroupBy(x => x.Exception.ToString())
            .OrderByDescending(grp => grp.Count());
        Console.WriteLine("Errors: ");
        foreach (var grp in distinctExceptions)
        {
            var count = grp.Count();
            Console.WriteLine($"[{count, 4}] {grp.Key}");
            Console.WriteLine("--------------------------------------------------------");
        }

        var opts = new FileStreamOptions()
        {
            Mode = FileMode.CreateNew,
            Access = FileAccess.Write,
            Share = FileShare.Read,
        };
        await using var changedSources = new StreamWriter(
            $"changed-source-paths-{sourceGmod.VisVersion}-{targetGmod.VisVersion}.txt",
            Encoding.UTF8,
            opts
        );
        await using var changedTargets = new StreamWriter(
            $"changed-target-paths-{sourceGmod.VisVersion}-{targetGmod.VisVersion}.txt",
            Encoding.UTF8,
            opts
        );

        foreach (var (sourcePath, targetPath) in context.ChangedPaths)
        {
            await changedSources.WriteLineAsync(sourcePath);
            await changedTargets.WriteLineAsync(targetPath);
        }

        await changedSources.FlushAsync();
        await changedTargets.FlushAsync();

        Assert.Empty(context.FailedConversions);
    }

    private record SmokeTestContext(Gmod TargetGmod, Channel<GmodPath> Channel)
    {
        public long Counter;
        public List<(GmodPath SourcePath, GmodPath? TargetPath, Exception Exception)> FailedConversions =
            new();
        public List<(string SourcePath, string TargetPath)> ChangedPaths = new();
    }
}

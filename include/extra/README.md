



```

    cli::progress bar("my example job", 10);
    while (!bar.done()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        bar.update();
    }

    bar.label("my other job");
    bar.reset();
    for (size_t i = 0; i < 4; i++, bar.update()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    bar.abort("ERROR!");
    exit(1);

```
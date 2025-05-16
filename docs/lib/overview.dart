import 'package:flutter/material.dart';
import 'package:flutter_localizations/flutter_localizations.dart';
import 'package:provider/provider.dart';
import 'package:syntax_highlight/syntax_highlight.dart';
import 'documentation_page.dart';
import 'package:flutter_gen/gen_l10n/app_localizations.dart';
import 'providers/locale_provider.dart';
import 'dart:js' as js;

// Класс для хранения примеров кода
class CodeExample {
  final String name;  
  final String code;
  final String description;
  
  CodeExample({required this.name, required this.code, this.description = ''});
}

class MonoScriptPage extends StatefulWidget {
  @override
  _MonoScriptPageState createState() => _MonoScriptPageState();
}

class _MonoScriptPageState extends State<MonoScriptPage> {
  List<CodeExample> _pageExamples = [];
  CodeExample? selectedExample;
  Highlighter? highlighter;
  String _currentLanguage = 'EN';

  void _initializeLocalizedExamples(BuildContext context) {
    final loc = AppLocalizations.of(context)!;

    final String? currentSelectedExampleName = selectedExample?.name;

    _pageExamples = [
      CodeExample(
        name: loc.exampleFibonacci, 
        description: loc.exampleFibonacciDesc, 
        code: '''[i64]fibonacci(i64: n) @strict @pure
|   if (n == 0)
|   |   return 0
|   else if (n == 1)
|   |   return 1
|   return fibonacci(n - 1->i64) + fibonacci(n - 2->i64)

[i32]main()
|   echo(toString_long(fibonacci(47))) // 2971215073
|   return 0''',
      ),
      CodeExample(
        name: loc.exampleHelloWorld,
        description: loc.exampleHelloWorldDesc,
        code: '''[void]hello_world() @strict @entry
|   echo("Hello World!!!")''',
      ),
      CodeExample(
        name: loc.exampleSort,
        description: loc.exampleSortDesc,
        code: '''[void]bubbleSort(array<i32>: arr)
|   n ^= len(arr)
|   for i in range(0, n)
|   |   for j in range(0, n - 1)
|   |   |   if arr[j] > arr[j + 1]
|   |   |   |   temp ^= arr[j]
|   |   |   |   arr[j] = arr[j + 1]
|   |   |   |   arr[j + 1] = temp

[string]getArray(array<i32>: arr)
|   result ^= ""
|   for i in range(0, len(arr))
|   |   result += arr[i]
|   return result

[void]main()
|   array<i32> nums = [5, 3, 1, 4, 2]
|   echo("Before sort:" + getArray(nums))
|
|   bubbleSort(nums)
|
|   echo("After sort:" + getArray(nums))''',
      ),
      CodeExample(
        name: loc.exampleGeneralCode,
        description: loc.exampleGeneralCodeDesc,
        code: '''[string]hello_str() @strict
|   string mark = " hz"
|   mark = "x" + mark + toString_int(2)
|   return "Hello" + mark

[i32]main() @entry
|   string concatString = hello_str() + " " + "World" + "!"
|   echo(concatString)
|   i1 isTrue = ((3>2)!=true)==0 // 1
|   i32 castedValue = -isTrue + 5 // -1 + 5
|   return castedValue // 4''',
      ),
      CodeExample(
        name: loc.exampleModules,
        description: loc.exampleModulesDesc,
        code: '''// math.ms
[i32]mySum(i32 : a, i32 : b)
|   return a + b

[i32]myMul(i32 : a, i32 : b)
|   return a * b

// moduleTest.ms
use
|>  math

[i32]moduleTest() @entry
|   // Test the math module
|   final a ^= 5
|   final b ^= 10
|   const i32 sum = mySum(a, b)
|   const i32 mul = myMul(a, b)
|   // Print the results
|   echo("a: " + toString_int(a) + ", b: " + toString_int(b))
|   echo("Sum: " + toString_int(sum) + ", Mul: " + toString_int(mul))
|   return 0'''
      )
    ];

    if (currentSelectedExampleName != null) {
      try {
        selectedExample = _pageExamples.firstWhere((ex) => ex.name == currentSelectedExampleName);
      } catch (e) {
        if (_pageExamples.isNotEmpty) {
          selectedExample = _pageExamples[0];
        } else {
          selectedExample = null;
        }
      }
    } else if (_pageExamples.isNotEmpty) {
      selectedExample = _pageExamples[0];
    } else {
      selectedExample = null;
    }
  }

  @override
  void initState() {
    super.initState();
    _initializeHighlighter();

    WidgetsBinding.instance.addPostFrameCallback((_) {
      if (mounted) 
      {
        final localeProvider = Provider.of<LocaleProvider>(context, listen: false);
        setState(() {
          _currentLanguage = localeProvider.locale.languageCode.toUpperCase();
        });
      }
    });
  }

  @override
  void didChangeDependencies() {
    super.didChangeDependencies();

    final localeProvider = Provider.of<LocaleProvider>(context, listen: false);
    final newLanguageCode = localeProvider.locale.languageCode.toUpperCase();
    if (_currentLanguage != newLanguageCode) {
      setState(() {
        _currentLanguage = newLanguageCode;
      });
    }

    _initializeLocalizedExamples(context);

    if (selectedExample == null && _pageExamples.isNotEmpty) {
       setState(() {
           selectedExample = _pageExamples[0];
       });
    }
  }
  
  Future<void> _initializeHighlighter() async {
    await Highlighter.initialize(['dart']); 
    var darkTheme = await HighlighterTheme.loadDarkTheme(); 
    setState(() {
      highlighter = Highlighter(
        language: 'dart', 
        theme: darkTheme,
      );
    });
  }
  
  Widget _getHighlightedCode(String code) {
    if (highlighter != null) {
      return Text.rich(
        highlighter!.highlight(code),
        style: TextStyle(
          fontFamily: 'Fira Code',
          fontSize: 16,
          height: 1.5,
          color: Colors.white,
        ),
        );
    } else {
      return Text(
        code,
        style: TextStyle(
          fontFamily: 'Fira Code',
          fontSize: 16,
          height: 1.5,
          color: Colors.white,
        ),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Color(0xFF0D1321),
      appBar: AppBar(
        backgroundColor: Colors.white,
        titleSpacing: 5,
        toolbarHeight: 56,
        title: Row(
          mainAxisAlignment: MainAxisAlignment.start,
          children: <Widget>[
            SizedBox(width: 67.5),
            Image(
              image: AssetImage('images/MONOSCRIPT_white.png'),
              height: 60,
              width: 235,
            ),
            Expanded(
              child: Row(
                mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                children: <Widget>[
                  _navBarItem(AppLocalizations.of(context)!.navOverview, isActive: true),
                  _navBarItem(AppLocalizations.of(context)!.navGetStarted),
                  _navBarItem(AppLocalizations.of(context)!.navDocs),
                  _navBarItem(AppLocalizations.of(context)!.navCommunity),
                  _navBarItem(AppLocalizations.of(context)!.navBlog),
                  // Разделитель и переключатель языка
                  Padding(
                    padding: const EdgeInsets.only(left: 24.0),
                    child: Row(
                      children: [
                        Container(
                          height: 24,
                          width: 1,
                          color: Color(0xFFCCCCCC),
                        ),
                        SizedBox(width: 24),
                        _languageSelector(),
                      ],
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
      body: SingleChildScrollView(
        child: IntrinsicHeight(
          child: Column(
            children: <Widget>[
              // Основной контент
              _welcomeContent(),

              _smallOverview(),

              _roadmapSection(), 

              _licenseSection(), 

              _actionButtons(),
              // Дополнительное пространство
              SizedBox(height: 40),

              // Нижняя хуйня
              _footer(),
            ],
          ),
        ),
      ),
    );
  }

Widget _welcomeContent() {
  return SingleChildScrollView(
    child: Row(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: <Widget>[
        // Основная часть
        Expanded(
          flex: 3,
          child: Container(
            margin: EdgeInsets.only(right: 16),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                // Заголовок и описание
                Padding(
                  padding: EdgeInsets.fromLTRB(160.0, 48.0, 160.0, 0.0), // было 80.0, 48.0, 24.0, 0.0
                  child: Row(  // Row для размещения заголовка и логотипа рядом
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      // Левая часть с заголовком
                      Expanded(
                        child: Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: <Widget>[
                            _buildTitle(),
                            
                            SizedBox(height: 16),
                            Text(
                              AppLocalizations.of(context)!.welcomeSlogan,
                              style: TextStyle( 
                                fontSize: 16,
                                fontStyle: FontStyle.italic,
                                color: Colors.white70,
                              ),
                            ),
                            SizedBox(height: 24),
                            _buildButtonRow(),
                            SizedBox(height: 40),
                                                        
                            Padding(
                              padding: EdgeInsets.fromLTRB(0, 0.0, 16.0, 16.0), // Такие же отступы как у блока кода ниже
                              child: Row(
                                crossAxisAlignment: CrossAxisAlignment.start,
                                children: [
                                  // Левая часть - примеры кода с шириной как у блока кода
                                  Expanded(
                                    flex: 9, // Такой же flex как у блока кода
                                    child: Container(
                                      padding: EdgeInsets.all(24),
                                      decoration: BoxDecoration(
                                        color: Color(0xFF0D1117).withOpacity(0.5),
                                        borderRadius: BorderRadius.circular(12),
                                        border: Border.all(
                                          color: Color(0xFF30363D),
                                          width: 1,
                                        ),
                                      ),
                                      child: Column(
                                        crossAxisAlignment: CrossAxisAlignment.start,
                                        children: [
                                          // Заголовок с иконкой
                                          Row(
                                            children: [
                                              Icon(Icons.code, size: 24, color: Colors.white),
                                              SizedBox(width: 16),
                                              Text(
                                                AppLocalizations.of(context)!.codeExamplesTitle,
                                                style: TextStyle(
                                                  fontSize: 22,
                                                  fontWeight: FontWeight.bold,
                                                  color: Colors.white,
                                                ),
                                              ),
                                            ],
                                          ),
                                          Divider(
                                            color: Color(0xFF30363D),
                                            height: 24,
                                          ),
                                          // Описание секции
                                          Text(
                                            AppLocalizations.of(context)!.codeExamplesDescription,
                                            style: TextStyle(
                                              fontSize: 16,
                                              color: Colors.white70,
                                              height: 1.5,
                                            ),
                                          ),
                                          SizedBox(height: 16),
                                          // Выбор примеров кода
                                          _buildExampleChips(),
                                        ],
                                      ),
                                    ),
                                  ),
                                  
                                  // Добавляем пустое пространство справа, чтобы сохранить пропорции как у блока кода
                                  SizedBox(width: 24),
                                  Expanded(flex: 3, child: SizedBox()),
                                ],
                              ),
                            ),

                          ],
                        ),
                      ),
                      
                      // Логотип и текст MONOPTICAL справа
                      Container(
                        // Убираем фиксированную ширину
                        width: MediaQuery.of(context).size.width * 0.3, // Адаптивная ширина - 30% экрана
                        padding: EdgeInsets.fromLTRB(0, 0, 0, 0),
                        child: Column(
                          crossAxisAlignment: CrossAxisAlignment.start,
                          children: [
                            Container(
                              padding: EdgeInsets.symmetric(horizontal: 12, vertical: 6),
                              decoration: BoxDecoration(
                                color: Colors.white,
                                borderRadius: BorderRadius.circular(6),
                                border: Border.all(
                                  color: Colors.black,
                                  width: 1,
                                ),
                              ),
                              child: Row(
                                mainAxisSize: MainAxisSize.min,
                                children: [
                                  Text(
                                    AppLocalizations.of(context)!.byLabel,
                                    style: TextStyle(
                                      fontSize: 16,
                                      fontWeight: FontWeight.w300,
                                      color: Colors.black54,
                                      letterSpacing: 0.5,
                                    ),
                                  ),
                                  Stack(
                                    alignment: Alignment.center,
                                    children: [
                                      // Текст MONOPTICAL
                                      Text(
                                        AppLocalizations.of(context)!.companyName,
                                        style: TextStyle(
                                          fontSize: 18,
                                          fontWeight: FontWeight.bold,
                                          color: const Color.fromARGB(255, 61, 58, 54),
                                          letterSpacing: 0.8,
                                        ),
                                      ),
                                      // Горизонтальная линия, проходящая через середину букв
                                      Positioned(
                                        top: 12.5, // Примерно половина высоты шрифта
                                        left: 0,
                                        right: 0,
                                        child: Container(
                                          height: 1.5, // Толщина линии
                                          color: const Color.fromARGB(178, 61, 58, 54),
                                        ),
                                      ),
                                    ],
                                  ),
                                  SizedBox(width: 6),
                                  Icon(Icons.verified, size: 16, color: Colors.black54)
                                ],
                              ),
                            ),
                            SizedBox(height: 8),
                            // Адаптивный контейнер для изображения
                            AspectRatio(
                              aspectRatio: 16/11, // Поддерживаем соотношение сторон
                              child: Container(
                                decoration: BoxDecoration(
                                  borderRadius: BorderRadius.circular(16),
                                  color: Colors.white,
                                  boxShadow: [
                                    BoxShadow(
                                      color: Colors.black26,
                                      blurRadius: 10,
                                      spreadRadius: 2,
                                      offset: Offset(0, 4),
                                    ),
                                  ],
                                ),
                                child: ClipRRect(
                                  borderRadius: BorderRadius.circular(16),
                                  child: Image(
                                    image: AssetImage("images/FullLogo.png"),
                                    fit: BoxFit.contain, // contain вместо cover для сохранения пропорций
                                  ),
                                ),
                              ),
                            ),
                            SizedBox(height: 16),
                          ],
                        ),
                      ),
                    ],
                  ),
                ),
                
                // Секция примера кода 
                Padding(
                  padding: EdgeInsets.fromLTRB(160.0, 24.0, 160.0, 16.0),
                  child: selectedExample != null && selectedExample!.description.isNotEmpty
                    ? Text(
                        selectedExample!.description,
                        style: TextStyle(
                          fontSize: 16,
                          fontStyle: FontStyle.italic,
                          color: Colors.white70,
                        ),
                      )
                    : SizedBox(),
                ),
                
                // Блок кода с фиксированной высотой
                Padding(
                  padding: EdgeInsets.fromLTRB(160.0, 0.0, 160.0, 24.0), 
                  child: Row(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      // Блок кода (слева)
                      Expanded(
                        flex: 7, // 70% места для кода
                        child: selectedExample != null
                          ? Container(
                              height: 750, // Фиксированная высота
                              decoration: BoxDecoration(
                                color: Color(0xFF0D1117),
                                borderRadius: BorderRadius.circular(8),
                                border: Border.all(color: Color(0xFF30363D)),
                                boxShadow: [
                                  BoxShadow(
                                    color: Colors.black.withOpacity(0.3),
                                    blurRadius: 8,
                                    offset: Offset(0, 2),
                                  ),
                                ],
                              ),
                              child: SingleChildScrollView(
                                padding: EdgeInsets.all(16),
                                child: Column(
                                  crossAxisAlignment: CrossAxisAlignment.start,
                                  children: [
                                    // Имитация строки с кнопками окна терминала
                                    Row(
                                      mainAxisSize: MainAxisSize.min,
                                      children: [
                                        Container(height: 12, width: 12, decoration: BoxDecoration(color: Colors.red, borderRadius: BorderRadius.circular(6))),
                                        SizedBox(width: 6),
                                        Container(height: 12, width: 12, decoration: BoxDecoration(color: Colors.amber, borderRadius: BorderRadius.circular(6))),
                                        SizedBox(width: 6),
                                        Container(height: 12, width: 12, decoration: BoxDecoration(color: Colors.green, borderRadius: BorderRadius.circular(6))),
                                        Spacer(),
                                        Text('monoscript', style: TextStyle(color: Colors.white70, fontSize: 12)),
                                      ],
                                    ),
                                    Divider(color: Color(0xFF30363D), height: 24),
                                    _getHighlightedCode(selectedExample!.code)
                                  ],
                                ),
                              ),
                            )
                          : SizedBox(),
                      ),

                      SizedBox(width: 24), // Отступ между блоками
                      // Блок Features (справа)
                      Expanded(
                        flex: 3, // 30% места для Features
                        child: SizedBox(
                          height: 750, // Фиксированная высота равная блоку кода
                          child: Column(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              // Консольный блок с фиксированной высотой
                              Container(
                                height: 260, // Фиксированная высота для консоли
                                padding: EdgeInsets.all(16),
                                decoration: BoxDecoration(
                                  color: Color(0xFF101010),
                                  borderRadius: BorderRadius.circular(8),
                                  border: Border.all(color: Color(0xFF333333), width: 1),
                                ),
                                child: Column(
                                  crossAxisAlignment: CrossAxisAlignment.start,
                                  children: [
                                    // Заголовок консоли
                                    Row(
                                      children: [
                                        Icon(Icons.terminal, color: Colors.greenAccent, size: 18),
                                        SizedBox(width: 8),
                                        Text(
                                          'Output',
                                          style: TextStyle(
                                            fontSize: 18,
                                            fontWeight: FontWeight.bold,
                                            color: Colors.greenAccent,
                                            fontFamily: 'Fira Code',
                                          ),
                                        ),
                                        Spacer(),
                                        Container(
                                          padding: EdgeInsets.symmetric(horizontal: 6, vertical: 2),
                                          decoration: BoxDecoration(
                                            color: Color(0xFF333333),
                                            borderRadius: BorderRadius.circular(4),
                                          ),
                                          child: Text(
                                            'monoscript',
                                            style: TextStyle(
                                              fontSize: 12,
                                              color: Colors.white70,
                                              fontFamily: 'Fira Code',
                                            ),
                                          ),
                                        ),
                                      ],
                                    ),
                                    SizedBox(height: 12),
                                    // Консоль с фиксированной высотой и скроллингом
                                    Expanded(
                                      child: Container(
                                        decoration: BoxDecoration(
                                          color: Color(0xFF0A0A0A),
                                          borderRadius: BorderRadius.circular(4),
                                          border: Border.all(color: Color(0xFF2A2A2A)),
                                        ),
                                        child: SingleChildScrollView(
                                          child: Padding(
                                            padding: EdgeInsets.all(12),
                                            child: Column(
                                              crossAxisAlignment: CrossAxisAlignment.start,
                                              children: [
                                                Row(
                                                  children: [
                                                    Text(
                                                      '\$ ',
                                                      style: TextStyle(
                                                        color: Colors.greenAccent,
                                                        fontWeight: FontWeight.bold,
                                                        fontFamily: 'Fira Code',
                                                      ),
                                                    ),
                                                    Text(
                                                      'monoscript run example.mono',
                                                      style: TextStyle(
                                                        color: Colors.white,
                                                        fontFamily: 'Fira Code',
                                                      ),
                                                    ),
                                                  ],
                                                ),
                                                SizedBox(height: 8),
                                                Padding(
                                                  padding: EdgeInsets.only(left: 16),
                                                  child: _getConsoleOutput(selectedExample),
                                                )
                                              ],
                                            ),
                                          ),
                                        ),
                                      ),
                                    ),
                                    SizedBox(height: 12),

                                    // Информация о производительности
                                    Text(
                                      AppLocalizations.of(context)!.executionTime(_getExecutionTime(selectedExample)),
                                      style: TextStyle(
                                        fontSize: 12,
                                        color: Colors.grey,
                                        fontFamily: 'Fira Code',
                                      ),
                                    ),
                                  ],
                                ),
                              ),

                              SizedBox(height: 16),

                              // Блок Features с фиксированной высотой и скроллингом
                              Expanded(
                                child: Container(
                                  padding: EdgeInsets.fromLTRB(16.0, 16.0, 16.0, 16.0),
                                  decoration: BoxDecoration(
                                    color: Color(0xFF0D1321),
                                    border: Border.all(
                                      color: Color(0xFF1A2231),
                                      width: 1,
                                    ),
                                    borderRadius: BorderRadius.circular(8),
                                    boxShadow: [
                                      BoxShadow(
                                        color: Colors.black12,
                                        blurRadius: 8,
                                        offset: Offset(0, 4),
                                      ),
                                    ],
                                  ),
                                  child: SingleChildScrollView(
                                    child: _buildCodeFeatureSection(),
                                  ),
                                ),
                              ),
                            ],
                          ),
                        ),
                      ),
                      ],
                  ),
                ),
              ],
            ),
          ),
        ),
      ],
    ),
  );
}

Widget _smallOverview() {
  return Container(
    padding: EdgeInsets.all(24),
    margin: EdgeInsets.fromLTRB(160, 20, 160, 0), // было 80, 20, 48, 0
    decoration: BoxDecoration(
      color: Color(0xFF0D1117).withOpacity(0.7),
      borderRadius: BorderRadius.circular(12),
      border: Border.all(
        color: Color(0xFF30363D),
        width: 1,
      ),
      boxShadow: [
        BoxShadow(
          color: Colors.black.withOpacity(0.2),
          blurRadius: 10,
          offset: Offset(0, 4),
        ),
      ],
    ),
    child: Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Row(
          children: [
            Icon(Icons.subject, size: 28, color: Colors.white),
            SizedBox(width: 16),
            Text(
              AppLocalizations.of(context)!.overviewSectionTitle,
              style: TextStyle(
                fontSize: 24,
                fontWeight: FontWeight.bold,
                color: Colors.white,
              ),
            ),
          ],
        ),
        Divider(
          color: Color(0xFF30363D),
          height: 24,
        ),
        SizedBox(height: 16),
        
        // Main description
        Text(
          AppLocalizations.of(context)!.overviewSectionDesc,
          style: TextStyle(
            fontSize: 16,
            color: Colors.white,
            height: 1.5,
          ),
        ),
        SizedBox(height: 12),
        
        // Performance
        Row(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Icon(Icons.speed, color: Colors.white, size: 24),
            SizedBox(width: 12),
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    AppLocalizations.of(context)!.performanceTitle,
                    style: TextStyle(
                      fontSize: 18,
                      fontWeight: FontWeight.bold,
                      color: Colors.white,
                    ),
                  ),
                  SizedBox(height: 4),
                  Text(
                    AppLocalizations.of(context)!.performanceDesc,
                    style: TextStyle(
                      fontSize: 14,
                      color: Colors.white70,
                      height: 1.5,
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
        SizedBox(height: 16),
        
        // Security
        Row(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Icon(Icons.security, color: Colors.white, size: 24),
            SizedBox(width: 12),
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    AppLocalizations.of(context)!.securityTitle,
                    style: TextStyle(
                      fontSize: 18,
                      fontWeight: FontWeight.bold,
                      color: Colors.white,
                    ),
                  ),
                  SizedBox(height: 4),
                  Text(
                    AppLocalizations.of(context)!.securityDesc,
                    style: TextStyle(
                      fontSize: 14,
                      color: Colors.white70,
                      height: 1.5,
                    ),
                  ),
                ],
              ),
            ),
          ],
        ),
        SizedBox(height: 24),
        
        // Supported paradigms
        Text(
          AppLocalizations.of(context)!.paradigmsTitle,
          style: TextStyle(
            fontSize: 20,
            fontWeight: FontWeight.bold,
            color: Colors.white,
          ),
        ),
        SizedBox(height: 12),
        
        // Paradigms table
        Container(
          decoration: BoxDecoration(
            color: Color(0xFF1A2231),
            borderRadius: BorderRadius.circular(8),
            border: Border.all(color: Color(0xFF2A3142), width: 1),
          ),
          child: Column(
            children: [
              _paradigmRow(AppLocalizations.of(context)!.paradigmImperative, AppLocalizations.of(context)!.paradigmImperativeDesc, true),
              _paradigmRow(AppLocalizations.of(context)!.paradigmFunctional, AppLocalizations.of(context)!.paradigmFunctionalDesc, false),
              _paradigmRow(AppLocalizations.of(context)!.paradigmModular, AppLocalizations.of(context)!.paradigmModularDesc, true),
              _paradigmRow(AppLocalizations.of(context)!.paradigmStructural, AppLocalizations.of(context)!.paradigmStructuralDesc, false),
            ],
          ),
        ),
      ],
    ),
  );
}

// Раздел о лицензии LGPL-2.1 и Open-Source
Widget _licenseSection() {
  return Container(
    padding: EdgeInsets.all(24),
    margin: EdgeInsets.fromLTRB(160, 0, 160, 0), // было 80, 0, 48, 0
    decoration: BoxDecoration(
      color: Color(0xFF0D1117).withOpacity(0.7),
      borderRadius: BorderRadius.circular(12),
      border: Border.all(
        color: Color(0xFF30363D),
        width: 1,
      ),
    ),
    child: Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Row(
          children: [
            Icon(Icons.gavel, size: 28, color: Colors.white),
            SizedBox(width: 16),
            Text(
              AppLocalizations.of(context)!.licenseSectionTitle,
              style: TextStyle(
                fontSize: 24,
                fontWeight: FontWeight.bold,
                color: Colors.white,
              ),
            ),
            Spacer(),
            Container(
              padding: EdgeInsets.symmetric(horizontal: 12, vertical: 6),
              decoration: BoxDecoration(
                color: Color(0xFF2E5C87),
                borderRadius: BorderRadius.circular(30),
              ),
              child: Text(
                'LGPL-2.1',
                style: TextStyle(
                  fontSize: 16,
                  fontWeight: FontWeight.bold,
                  color: Colors.white,
                ),
              ),
            ),
          ],
        ),
        Divider(
          color: Color(0xFF30363D),
          height: 24,
        ),
        SizedBox(height: 16),
        
        // Main description
        Text(
          AppLocalizations.of(context)!.licenseDescription,
          style: TextStyle(
            fontSize: 16,
            color: Colors.white,
            height: 1.5,
          ),
        ),
        SizedBox(height: 16),
        
        // Benefits grid
        Row(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            // First column
            Expanded(
              child: _licenseBenefitItem(
                Icons.handshake, 
                AppLocalizations.of(context)!.benefitCollabTitle, 
                AppLocalizations.of(context)!.benefitCollabDesc
              ),
            ),
            SizedBox(width: 16),
            // Second column
            Expanded(
              child: _licenseBenefitItem(
                Icons.integration_instructions, 
                AppLocalizations.of(context)!.benefitIntegrationTitle, 
                AppLocalizations.of(context)!.benefitIntegrationDesc
              ),
            ),
            SizedBox(width: 16),
            // Third column
            Expanded(
              child: _licenseBenefitItem(
                Icons.lock_open, 
                AppLocalizations.of(context)!.benefitModifyTitle,
                AppLocalizations.of(context)!.benefitModifyDesc
              ),
            ),
          ],
        ),
        
        SizedBox(height: 16),
        Container(
          padding: EdgeInsets.all(12),
          decoration: BoxDecoration(
            color: Color(0xFF1A2231),
            borderRadius: BorderRadius.circular(8),
          ),
          child: Row(
            children: [
              Icon(Icons.favorite, color: Colors.white, size: 20),
              SizedBox(width: 12),
              Expanded(
                child: Text(
                  AppLocalizations.of(context)!.openSourceBelief,
                  style: TextStyle(
                    fontSize: 14,
                    fontStyle: FontStyle.italic,
                    color: Colors.white70,
                  ),
                ),
              ),
            ],
          ),
        ),
      ],
    ),
  );
}

// Helper for license benefits
Widget _licenseBenefitItem(IconData icon, String title, String description) {
  return Column(
    crossAxisAlignment: CrossAxisAlignment.start,
    children: [
      Row(
        children: [
          Icon(icon, color: Colors.white, size: 20),
          SizedBox(width: 8),
          Text(
            title,
            style: TextStyle(
              fontSize: 18,
              fontWeight: FontWeight.bold,
              color: Colors.white,
            ),
          ),
        ],
      ),
      SizedBox(height: 8),
      Text(
        description,
        style: TextStyle(
          fontSize: 14,
          color: Colors.white70,
        ),
      ),
    ],
  );
}

// Раздел с мини-RoadMap
Widget _roadmapSection() {
  return Container(
    padding: EdgeInsets.all(24),
    margin: EdgeInsets.fromLTRB(160, 24, 160, 24), // было 80, 24, 48, 24
    decoration: BoxDecoration(
      color: Color(0xFF0D1117).withOpacity(0.7),
      borderRadius: BorderRadius.circular(12),
      border: Border.all(
        color: Color(0xFF30363D),
        width: 1,
      ),
    ),
    child: Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Row(
          children: [
            Icon(Icons.timeline, size: 28, color: Colors.white),
            SizedBox(width: 16),
            Text(
              AppLocalizations.of(context)!.roadmapSectionTitle,
              style: TextStyle(
                fontSize: 24,
                fontWeight: FontWeight.bold,
                color: Colors.white,
              ),
            ),
            SizedBox(width: 16),
            Container(
              padding: EdgeInsets.symmetric(horizontal: 12, vertical: 6),
              decoration: BoxDecoration(
                color: Color(0xFF1E4422),
                borderRadius: BorderRadius.circular(30),
              ),
              child: Text(
                AppLocalizations.of(context)!.roadmapStatus,
                style: TextStyle(
                  fontSize: 14,
                  fontWeight: FontWeight.bold,
                  color: Colors.white,
                ),
              ),
            ),
          ],
        ),
        Divider(
          color: Color(0xFF30363D),
          height: 24,
        ),
        SizedBox(height: 16),
        
        Text(
          AppLocalizations.of(context)!.roadmapDescription,
          style: TextStyle(
            fontSize: 16,
            color: Colors.white,
            height: 1.5,
          ),
        ),
        SizedBox(height: 24),
        
        // Timeline visualization
        Row(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            _roadmapItem(AppLocalizations.of(context)!.roadmapQ22025, AppLocalizations.of(context)!.roadmapQ22025Milestone, AppLocalizations.of(context)!.roadmapQ22025Status, true),
            _roadmapItem(AppLocalizations.of(context)!.roadmapQ32025, AppLocalizations.of(context)!.roadmapQ32025Milestone, AppLocalizations.of(context)!.roadmapQ32025Status),
            _roadmapItem(AppLocalizations.of(context)!.roadmapQ42025, AppLocalizations.of(context)!.roadmapQ42025Milestone, AppLocalizations.of(context)!.roadmapQ42025Status),
            _roadmapItem(AppLocalizations.of(context)!.roadmapQ12026, AppLocalizations.of(context)!.roadmapQ12026Milestone, AppLocalizations.of(context)!.roadmapQ12026Status),
          ],
        ),
        
        SizedBox(height: 16),
        Container(
          padding: EdgeInsets.all(12),
          decoration: BoxDecoration(
            color: Color(0xFF1A2231),
            borderRadius: BorderRadius.circular(8),
          ),
          child: Row(
            children: [
              Icon(Icons.lightbulb, color: Colors.white, size: 20),
              SizedBox(width: 12),
              Expanded(
                child: Text(
                  AppLocalizations.of(context)!.roadmapFeedback,
                  style: TextStyle(
                    fontSize: 14,
                    fontStyle: FontStyle.italic,
                    color: Colors.white70,
                  ),
                ),
              ),
            ],
          ),
        ),
      ],
    ),
  );
}

// Helper for roadmap items
Widget _roadmapItem(String quarter, String milestone, String status, [bool isCurrent = false]) {
  return Expanded(
    child: Container(
      margin: EdgeInsets.symmetric(horizontal: 4),
      child: Column(
        children: [
          Container(
            padding: EdgeInsets.symmetric(vertical: 8),
            decoration: BoxDecoration(
              color: isCurrent ? Color(0xFF1E4422) : Color(0xFF1A2231),
              borderRadius: BorderRadius.circular(8),
            ),
            child: Center(
              child: Text(
                quarter,
                style: TextStyle(
                  fontSize: 16,
                  fontWeight: FontWeight.bold,
                  color: Colors.white,
                ),
              ),
            ),
          ),
          Container(
            width: 2,
            height: 30,
            color: isCurrent ? Color(0xFF1E4422) : Color(0xFF1A2231),
          ),
          Container(
            height: 16,
            width: 16,
            decoration: BoxDecoration(
              color: isCurrent ? Color(0xFF1E4422) : Color(0xFF1A2231),
              shape: BoxShape.circle,
              border: Border.all(
                color: isCurrent ? Colors.white : Color(0xFF30363D),
                width: 2,
              ),
            ),
          ),
          SizedBox(height: 8),
          Text(
            milestone,
            textAlign: TextAlign.center,
            style: TextStyle(
              fontSize: 16,
              fontWeight: FontWeight.bold,
              color: Colors.white,
            ),
          ),
          SizedBox(height: 4),
          Text(
            status,
            textAlign: TextAlign.center,
            style: TextStyle(
              fontSize: 14,
              color: isCurrent ? Colors.greenAccent : Colors.white70,
              fontStyle: isCurrent ? FontStyle.normal : FontStyle.italic,
            ),
          ),
        ],
      ),
    ),
  );
}

Widget _actionButtons() {
  return Container(
    padding: EdgeInsets.fromLTRB(160, 32, 160, 16), // было 80, 32, 48, 16
    child: Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        // Верхний ряд кнопок: Documentation и Get Started
        Row(
          children: [
            // Кнопка Documentation (левая верхняя)
            Expanded(
              child: Container(
                margin: EdgeInsets.only(bottom: 16, right: 12),
                child: ElevatedButton(
                  onPressed: () {
                    Navigator.push(
                      context,
                      MaterialPageRoute(
                        builder: (context) => DocumentationPage(),
                      ),
                    );
                  },
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.white,
                    foregroundColor: Colors.black,
                    elevation: 3,
                    padding: EdgeInsets.all(0),
                    shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(8),
                    ),
                  ),
                  child: Container(
                    padding: EdgeInsets.all(24),
                    child: Row(
                      children: [
                        Icon(Icons.menu_book, size: 28),
                        SizedBox(width: 16),
                        Expanded(
                          child: Column(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              Text(
                                AppLocalizations.of(context)!.actionDocsTitle,
                                style: TextStyle(
                                  fontSize: 20,
                                  fontWeight: FontWeight.bold,
                                ),
                              ),
                              SizedBox(height: 6),
                              Text(
                                AppLocalizations.of(context)!.actionDocsDesc,
                                style: TextStyle(
                                  fontSize: 14,
                                  color: Colors.black87,
                                ),
                              ),
                            ],
                          ),
                        ),
                        Icon(Icons.arrow_forward, size: 24),
                      ],
                    ),
                  ),
                ),
              ),
            ),
            
            // Кнопка Get Started (правая верхняя)
            Expanded(
              child: Container(
                margin: EdgeInsets.only(bottom: 16, left: 12),
                child: ElevatedButton(
                  onPressed: () {},
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Color(0xFF17285A),
                    foregroundColor: Colors.white,
                    elevation: 3,
                    padding: EdgeInsets.all(0),
                    shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(8),
                    ),
                  ),
                  child: Container(
                    padding: EdgeInsets.all(24),
                    child: Row(
                      children: [
                        Icon(Icons.rocket_launch, size: 28),
                        SizedBox(width: 16),
                        Expanded(
                          child: Column(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              Text(
                                AppLocalizations.of(context)!.actionGetStartedTitle,
                                style: TextStyle(
                                  fontSize: 20,
                                  fontWeight: FontWeight.bold,
                                ),
                              ),
                              SizedBox(height: 6),
                              Text(
                                AppLocalizations.of(context)!.actionGetStartedDesc,
                                style: TextStyle(
                                  fontSize: 14,
                                  color: Colors.white70,
                                ),
                              ),
                            ],
                          ),
                        ),
                        Icon(Icons.arrow_forward, size: 24),
                      ],
                    ),
                  ),
                ),
              ),
            ),
          ],
        ),
        
        // Нижний ряд кнопок: Contribute и Blog
        Row(
          children: [
            // Кнопка Contribute (левая нижняя)
            Expanded(
              child: Container(
                margin: EdgeInsets.only(right: 12),
                child: ElevatedButton(
                  onPressed: () {
                    js.context.callMethod('open', ['https://github.com/MONOPTICAL/MONOSCRIPT']);
                  },
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Color(0xFF222940),
                    foregroundColor: Colors.white,
                    elevation: 3,
                    padding: EdgeInsets.all(0),
                    shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(8),
                    ),
                  ),
                  child: Container(
                    padding: EdgeInsets.all(24),
                    child: Row(
                      children: [
                        Icon(Icons.code, size: 28),
                        SizedBox(width: 16),
                        Expanded(
                          child: Column(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              Text(
                                AppLocalizations.of(context)!.actionContributeTitle,
                                style: TextStyle(
                                  fontSize: 20,
                                  fontWeight: FontWeight.bold,
                                ),
                              ),
                              SizedBox(height: 6),
                              Text(
                                AppLocalizations.of(context)!.actionContributeDesc,
                                style: TextStyle(
                                  fontSize: 14,
                                  color: Colors.white70,
                                ),
                              ),
                            ],
                          ),
                        ),
                        Icon(Icons.arrow_forward, size: 24),
                      ],
                    ),
                  ),
                ),
              ),
            ),
            
            // Кнопка Blog (правая нижняя)
            Expanded(
              child: Container(
                margin: EdgeInsets.only(left: 12),
                child: ElevatedButton(
                  onPressed: () {},
                  style: ElevatedButton.styleFrom(
                    backgroundColor: Color(0xFF281D30),
                    foregroundColor: Colors.white,
                    elevation: 3,
                    padding: EdgeInsets.all(0),
                    shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(8),
                    ),
                  ),
                  child: Container(
                    padding: EdgeInsets.all(24),
                    child: Row(
                      children: [
                        Icon(Icons.article, size: 28),
                        SizedBox(width: 16),
                        Expanded(
                          child: Column(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              Text(
                                AppLocalizations.of(context)!.actionBlogTitle,
                                style: TextStyle(
                                  fontSize: 20,
                                  fontWeight: FontWeight.bold,
                                ),
                              ),
                              SizedBox(height: 6),
                              Text(
                                AppLocalizations.of(context)!.actionBlogDesc,
                                style: TextStyle(
                                  fontSize: 14,
                                  color: Colors.white70,
                                ),
                              ),
                            ],
                          ),
                        ),
                        Icon(Icons.arrow_forward, size: 24),
                      ],
                    ),
                  ),
                ),
              ),
            ),
          ],
        ),
        
        SizedBox(height: 12),
        Text(
          AppLocalizations.of(context)!.actionWarning,
          style: TextStyle(
            fontSize: 12,
            fontStyle: FontStyle.italic,
            color: Colors.white70,
          ),
        ),
      ],
    ),
  );
}

Widget _footer() {
  return Container(
    width: double.infinity,
    padding: EdgeInsets.fromLTRB(160, 40, 160, 40),
    color: Color(0xFF0A0E19),
    child: Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Row(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Row(
                  crossAxisAlignment: CrossAxisAlignment.baseline,
                  textBaseline: TextBaseline.alphabetic,
                  children: [
                    Container(
                      padding: EdgeInsets.symmetric(horizontal: 8, vertical: 4),
                      decoration: BoxDecoration(
                        color: Colors.white,
                        borderRadius: BorderRadius.circular(6),
                      ),
                      child: Row(
                        crossAxisAlignment: CrossAxisAlignment.baseline,
                        textBaseline: TextBaseline.alphabetic,
                        children: [
                          Text(
                            'MONO',
                            style: TextStyle(
                              fontSize: 22,
                              fontWeight: FontWeight.bold,
                              color: Colors.black,
                              height: 1.0,
                            ),
                          ),
                          Text(
                            'script',
                            style: TextStyle(
                              fontSize: 18,
                              fontWeight: FontWeight.normal,
                              color: Colors.black,
                            ),
                          ),
                        ],
                      ),
                    )
                  ],
                ),
                SizedBox(height: 8),
                Text(
                  AppLocalizations.of(context)!.footerSlogan,
                  style: TextStyle(
                    color: Colors.white70,
                    fontSize: 14,
                  ),
                ),
              ],
            ),
            Spacer(),
            // Footer Navigation
            Row(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                _footerNavSection(
                  AppLocalizations.of(context)!.footerDocumentationTitle,
                  [AppLocalizations.of(context)!.footerDocLink1, AppLocalizations.of(context)!.footerDocLink2, AppLocalizations.of(context)!.footerDocLink3, AppLocalizations.of(context)!.footerDocLink4],
                ),
                SizedBox(width: 64),
                _footerNavSection(
                  AppLocalizations.of(context)!.footerCommunityTitle,
                  [AppLocalizations.of(context)!.footerCommunityLink1, AppLocalizations.of(context)!.footerCommunityLink2, AppLocalizations.of(context)!.footerCommunityLink3, AppLocalizations.of(context)!.footerCommunityLink4],
                ),
                SizedBox(width: 64),
                _footerNavSection(
                  AppLocalizations.of(context)!.footerResourcesTitle,
                  [AppLocalizations.of(context)!.footerResourcesLink1, AppLocalizations.of(context)!.footerResourcesLink2, AppLocalizations.of(context)!.footerResourcesLink3, AppLocalizations.of(context)!.footerResourcesLink4],
                ),
              ],
            ),
          ],
        ),
        
        SizedBox(height: 40),
        Divider(color: Color(0xFF1A2231), thickness: 1),
        SizedBox(height: 24),
        
        Row(
          children: [
            Text(
              AppLocalizations.of(context)!.footerCopyright,
              style: TextStyle(
                color: Colors.white60,
                fontSize: 14,
              ),
            ),
            Spacer(),
            Row(
              children: [
                _socialIcon(Icons.alternate_email, 'Email'),
                SizedBox(width: 16),
                _socialIcon(Icons.discord, 'Discord'),
                SizedBox(width: 16),
                _socialIcon(Icons.cloud, 'GitHub'),
                SizedBox(width: 16),
                _socialIcon(Icons.language, AppLocalizations.of(context)!.tooltipWebsite),
              ],
            ),
          ],
        ),
        
        SizedBox(height: 24),
        Row(
          children: [
            Text(
              AppLocalizations.of(context)!.securityInMindText,
              style: TextStyle(
                color: Colors.white38,
                fontSize: 12,
              ),
            ),
            SizedBox(width: 8),
            Icon(
              Icons.security,
              color: Colors.white,
              size: 14,
            ),
            Spacer(),
            Text(
              AppLocalizations.of(context)!.versionText,
              style: TextStyle(
                color: Colors.white,
                fontSize: 12,
                fontWeight: FontWeight.bold,
              ),
            ),
          ],
        ),
      ],
    ),
  );
}

Widget _footerNavSection(String title, List<String> links) {
  return Column(
    crossAxisAlignment: CrossAxisAlignment.start,
    children: [
      Text(
        title,
        style: TextStyle(
          color: Colors.white,
          fontSize: 18,
          fontWeight: FontWeight.bold,
        ),
      ),
      SizedBox(height: 16),
      ...links.map((link) => Padding(
        padding: EdgeInsets.only(bottom: 8),
        child: InkWell(
          onTap: () {},
          child: Text(
            link,
            style: TextStyle(
              color: Colors.white70,
              fontSize: 14,
            ),
          ),
        ),
      )),
    ],
  );
}

Widget _socialIcon(IconData icon, String tooltip) {
  return Tooltip(
    message: tooltip,
    child: InkWell(
      onTap: () {},
      child: Container(
        padding: EdgeInsets.all(8),
        decoration: BoxDecoration(
          color: Color(0xFF1A2231),
          borderRadius: BorderRadius.circular(8),
        ),
        child: Icon(
          icon,
          color: Colors.white70,
          size: 18,
        ),
      ),
    ),
  );
}

Widget _paradigmRow(String name, String description, bool isAlternate) {
  return Container(
    padding: EdgeInsets.symmetric(horizontal: 16, vertical: 12),
    decoration: BoxDecoration(
      color: isAlternate ? Color(0xFF21293B) : Colors.transparent,
      border: Border(
        bottom: BorderSide(
          color: Color(0xFF2A3142),
          width: 1,
        ),
      ),
    ),
    child: Row(
      children: [
        SizedBox(
          width: 150,
          child: Text(
            name,
            style: TextStyle(
              fontSize: 16,
              fontWeight: FontWeight.bold,
              color: Colors.white,
            ),
          ),
        ),
        Expanded(
          child: Text(
            description,
            style: TextStyle(
              fontSize: 14,
              color: Colors.white70,
            ),
          ),
        ),
      ],
    ),
  );
}
  
Widget _buildTitle() {
  return Row(
    crossAxisAlignment: CrossAxisAlignment.baseline,
    textBaseline: TextBaseline.alphabetic,
    children: [
      if (_currentLanguage == 'EN')
      ...[
        Text(
          'The ',
          style: TextStyle(
            fontSize: 35,
            fontWeight: FontWeight.normal,
            color: Colors.white,
          ),
        ),
        Row(
          crossAxisAlignment: CrossAxisAlignment.baseline,
          textBaseline: TextBaseline.alphabetic,
          children: [
            Container(
              padding: EdgeInsets.symmetric(horizontal: 8, vertical: 4),
              decoration: BoxDecoration(
                color: Colors.white,
                borderRadius: BorderRadius.circular(6),
              ),
              child: Row(
                crossAxisAlignment: CrossAxisAlignment.baseline,
                textBaseline: TextBaseline.alphabetic,
                children: [
                  Text(
                    'MONO',
                    style: TextStyle(
                      fontSize: 35,
                      fontWeight: FontWeight.w800,
                      color: Colors.black,
                      height: 1.0,
                    ),
                  ),
                  Text(
                    'script',
                    style: TextStyle(
                      fontSize: 25,
                      fontWeight: FontWeight.normal,
                      color: Colors.black,
                    ),
                  ),
                ],
              ),
            )
          ],
        ),
        Text(
          ' Programming Language',
          style: TextStyle(
            fontSize: 35,
            fontWeight: FontWeight.normal,
            color: Colors.white,
          ),
        ),
      ]
      else
      ...[
        Text(
          'Язык программирования ',
          style: TextStyle(
            fontSize: 35,
            fontWeight: FontWeight.normal,
            color: Colors.white,
          ),
        ),
        Row(
          crossAxisAlignment: CrossAxisAlignment.baseline,
          textBaseline: TextBaseline.alphabetic,
          children: [
            Container(
              padding: EdgeInsets.symmetric(horizontal: 8, vertical: 4),
              decoration: BoxDecoration(
                color: Colors.white,
                borderRadius: BorderRadius.circular(6),
              ),
              child: Row(
                crossAxisAlignment: CrossAxisAlignment.baseline,
                textBaseline: TextBaseline.alphabetic,
                children: [
                  Text(
                    'MONO',
                    style: TextStyle(
                      fontSize: 35,
                      fontWeight: FontWeight.w800,
                      color: Colors.black,
                      height: 1.0,
                    ),
                  ),
                  Text(
                    'script',
                    style: TextStyle(
                      fontSize: 25,
                      fontWeight: FontWeight.normal,
                      color: Colors.black,
                    ),
                  ),
                ],
              ),
            )
          ],
        ),
      ],
      Transform.translate(
        offset: Offset(5, -20), // Смещение вверх
        child: Container(
          padding: EdgeInsets.symmetric(horizontal: 6, vertical: 2),
          decoration: BoxDecoration(
            color: Colors.white,
            borderRadius: BorderRadius.circular(4),
          ),
          child: Text(
            AppLocalizations.of(context)!.betaVersion,
            style: TextStyle(
              fontSize: 12, // Уменьшенный размер шрифта
              fontWeight: FontWeight.bold,
              color: Colors.black,
            ),
          ),
        ),
      ),
    ],
  );
}
  
  Widget _buildButtonRow() {
    return Row(
      children: <Widget>[
        ElevatedButton.icon(
          icon: Icon(Icons.code),
          label: Text(AppLocalizations.of(context)!.githubButtonLabel),
          onPressed: () {
            js.context.callMethod('open', ['https://github.com/MONOPTICAL/MONOSCRIPT']);
          },
          style: ElevatedButton.styleFrom(
            backgroundColor: Color(0xFF252A34),
            foregroundColor: Colors.white,
            elevation: 2,
            padding: EdgeInsets.symmetric(horizontal: 20, vertical: 12),
            shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(8),
            ),
          ),
        ),
        SizedBox(width: 16),
        ElevatedButton.icon(
          icon: Icon(Icons.download_for_offline),
          label: Text(AppLocalizations.of(context)!.downloadButtonLabel),
          onPressed: () {},
          style: ElevatedButton.styleFrom(
            backgroundColor: Color(0xFF17285A),
            foregroundColor: Colors.white,
            elevation: 2,
            padding: EdgeInsets.symmetric(horizontal: 20, vertical: 12),
            shape: RoundedRectangleBorder(
              borderRadius: BorderRadius.circular(8),
            ),
          ),
        ),
      ],
    );
  }
  
  Widget _buildExampleChips() {
    return Wrap(
      spacing: 12,
      runSpacing: 12,
      children: _pageExamples.map((example) {
        return ChoiceChip(
          label: Text(example.name),
          selected: selectedExample == example,
          onSelected: (selected) {
            setState(() {
              selectedExample = example;
            });
          },
          selectedColor: Colors.white,
          backgroundColor: Color.fromARGB(255, 5, 1, 1),
          checkmarkColor: Colors.black,
          padding: EdgeInsets.symmetric(horizontal: 12, vertical: 8),
          labelStyle: TextStyle(
            color: selectedExample == example 
                ? Colors.black 
                : Colors.white,
            fontWeight: selectedExample == example 
                ? FontWeight.bold 
                : FontWeight.normal,
          ),
        );
      }).toList(),
    );
  }

Widget _languageSelector() {
  return Container(
    decoration: BoxDecoration(
      color: Color(0xFFF2F2F2),
      borderRadius: BorderRadius.circular(4),
    ),
    child: Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        // Английский язык
        InkWell(
          onTap: () {
            final localeProvider = Provider.of<LocaleProvider>(context, listen: false);
            localeProvider.setLocale(const Locale('en', ''));
            setState(() {
              _currentLanguage = 'EN';
            });
          },
          child: Container(
            padding: EdgeInsets.symmetric(horizontal: 10, vertical: 6),
            decoration: BoxDecoration(
              color: _currentLanguage == 'EN' ? Color(0xFF333333) : Colors.transparent,
              borderRadius: BorderRadius.horizontal(left: Radius.circular(4)),
            ),
            child: Text(
              AppLocalizations.of(context)!.languageEN,
              style: TextStyle(
                color: _currentLanguage == 'EN' ? Colors.white : Color(0xFF333333),
                fontWeight: FontWeight.bold,
                fontSize: 12,
              ),
            ),
          ),
        ),
        // Русский язык
        InkWell(
          onTap: () {
            final localeProvider = Provider.of<LocaleProvider>(context, listen: false);
            localeProvider.setLocale(const Locale('ru', ''));
            setState(() {
              _currentLanguage = 'RU';
            });
          },
          child: Container(
            padding: EdgeInsets.symmetric(horizontal: 10, vertical: 6),
            decoration: BoxDecoration(
              color: _currentLanguage == 'RU' ? Color(0xFF333333) : Colors.transparent,
              borderRadius: BorderRadius.horizontal(right: Radius.circular(4)),
            ),
            child: Text(
              AppLocalizations.of(context)!.languageRU,
              style: TextStyle(
                color: _currentLanguage == 'RU' ? Colors.white : Color(0xFF333333),
                fontWeight: FontWeight.bold,
                fontSize: 12,
              ),
            ),
          ),
        ),
      ],
    ),
  );
}

Widget _buildCodeFeatureSection() {
  List<Map<String, String>> features = [];
  
  if (selectedExample != null) {
    switch (selectedExample!.name) {
      case 'Fibonacci' || 'Фибоначчи':
        features = [
          {
            'title': AppLocalizations.of(context)!.featureRecursionTitle,
            'description': AppLocalizations.of(context)!.featureRecursionDesc
          },
          {
            'title': AppLocalizations.of(context)!.featureTypeCastingTitle,
            'description': AppLocalizations.of(context)!.featureTypeCastingDesc
          },
          {
            'title': AppLocalizations.of(context)!.featurePureAnnotationTitle,
            'description': AppLocalizations.of(context)!.featurePureAnnotationDesc
          },
          {
            'title': AppLocalizations.of(context)!.featureStrictAnnotationTitle,
            'description': AppLocalizations.of(context)!.featureStrictAnnotationDesc
          },
        ];
        break;
      
      case 'Hello World' || 'Привет, мир':
        features = [
          {
            'title': AppLocalizations.of(context)!.featureEntryPointTitle,
            'description': AppLocalizations.of(context)!.featureEntryPointDesc
          },
          {
            'title': AppLocalizations.of(context)!.featureVoidReturnTypeTitle,
            'description': AppLocalizations.of(context)!.featureVoidReturnTypeDesc
          },
          {
            'title': AppLocalizations.of(context)!.featureEchoStatementTitle,
            'description': AppLocalizations.of(context)!.featureEchoStatementDesc
          },
          {
            'title': AppLocalizations.of(context)!.featureStringLiteralsTitle,
            'description': AppLocalizations.of(context)!.featureStringLiteralsDesc
          },
        ];
        break;
      
      case 'Sort' || 'Сортировка':
        features = [
          {
            'title': AppLocalizations.of(context)!.featureArrayIndexingTitle,
            'description': AppLocalizations.of(context)!.featureArrayIndexingDesc
          },
          {
            'title': AppLocalizations.of(context)!.featureForInLoopTitle,
            'description': AppLocalizations.of(context)!.featureForInLoopDesc
          },
          {
            'title': AppLocalizations.of(context)!.featureInitAssignTitle,
            'description': AppLocalizations.of(context)!.featureInitAssignDesc
          },
          {
            'title': AppLocalizations.of(context)!.featureArrayManipulationDesc,
            'description': AppLocalizations.of(context)!.featureArrayManipulationDesc
          },
        ];
        break;
      
      case 'General Code' || 'Общий код':
        features = [
          {
            'title': AppLocalizations.of(context)!.featureStringConcatTitle,
            'description': AppLocalizations.of(context)!.featureStringConcatDesc
          },
          {
            'title': AppLocalizations.of(context)!.featureBooleanOpsTitle,
            'description': AppLocalizations.of(context)!.featureBooleanOpsDesc
          },
          {
            'title': AppLocalizations.of(context)!.featureTypeConversionTitle,
            'description': AppLocalizations.of(context)!.featureTypeConversionDesc
          },
          {
            'title': AppLocalizations.of(context)!.featureMultipleFunctionsTitle,
            'description': AppLocalizations.of(context)!.featureMultipleFunctionsDesc
          },
        ];
      case 'Modules' || 'Модули':
        features = [
          {
            'title': AppLocalizations.of(context)!.featureModuleDeclTitle,
            'description': AppLocalizations.of(context)!.featureModuleDeclDesc
          },
          {
            'title': AppLocalizations.of(context)!.featureFinalVarsTitle,
            'description': AppLocalizations.of(context)!.featureFinalVarsDesc
          },
          {
            'title': AppLocalizations.of(context)!.featureModuleTestingTitle,
            'description': AppLocalizations.of(context)!.featureModuleTestingDesc
          },
          {
            'title': AppLocalizations.of(context)!.featureEntryPointTitle,
            'description': AppLocalizations.of(context)!.featureEntryPointDesc
          },
        ];
        break;
    }
  }
  
  return Column(
    crossAxisAlignment: CrossAxisAlignment.start,
    children: [
      Text(
        'Code Features',
        style: TextStyle(
          fontSize: 22, 
          fontWeight: FontWeight.bold, 
          color: Colors.white
        ),
      ),
      SizedBox(height: 16),
      ...features.map((feature) => _featureItem(
        feature['title']!, 
        feature['description']!
      )),
    ],
  );
}
  
  Widget _featureItem(String title, String description) {
    return Container(
      margin: EdgeInsets.only(bottom: 16),
      padding: EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: Color(0xFF1A2231),
        borderRadius: BorderRadius.circular(8),
        border: Border.all(color: Color(0xFF2A3142), width: 1),
      ),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Icon(Icons.check_circle, color: Colors.white, size: 20),
          SizedBox(width: 12),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  title,
                  style: TextStyle(
                    fontSize: 18,
                    fontWeight: FontWeight.bold,
                    color: Colors.white,
                  ),
                ),
                SizedBox(height: 4),
                Text(
                  description,
                  style: TextStyle(
                    fontSize: 14,
                    color: Colors.white70,
                  ),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }

Widget _navBarItem(String title, {bool isActive = false}) {
  String firstPart = '';
  String secondPart = '';
  
  switch(title) {
    case 'Overview':
      firstPart = 'OVER';
      secondPart = 'view';
      break;
    case 'Get Started':
      firstPart = 'GET';
      secondPart = 'started';
      break;
    case 'Docs':
      firstPart = 'DOC';
      secondPart = 's';
      break;
    case 'Community':
      firstPart = 'COMM';
      secondPart = 'unity';
      break;
    case 'Blog':
      firstPart = 'BLOG';
      secondPart = '';
      break;
    case 'Обзор':
      firstPart = 'ОБЗ';
      secondPart = 'ор';
      break;
    case 'Начать':
      firstPart = 'НАЧ';
      secondPart = 'ать';
      break;
    case 'Документация':
      firstPart = 'ДОК';
      secondPart = 'ументация';
      break;
    case 'Сообщество':
      firstPart = 'СОБ';
      secondPart = 'щества';
      break;
    case 'Блог':
      firstPart = 'БЛОГ';
      secondPart = '';
      break;
    default:
      firstPart = title;
      secondPart = '';
  }
  
  return Padding(
    padding: const EdgeInsets.symmetric(horizontal: 12.0),
    child: MouseRegion(
      cursor: SystemMouseCursors.click,
      child: AnimatedContainer(
        duration: Duration(milliseconds: 200),
        padding: EdgeInsets.symmetric(horizontal: 16.0, vertical: 8.0),
        decoration: BoxDecoration(
          border: Border(
            bottom: BorderSide(
              color: isActive ? Color(0xFF1A1A1A) : Colors.transparent,
              width: 2,
            ),
          ),
        ),
        child: InkWell(
          onTap: () {
            switch (title) {
              case 'Overview' || 'Обзор':
                break;
              case 'Get Started' || 'Начать':
                break;
              case 'Docs' || 'Документация':
                Navigator.push(
                  context,
                  MaterialPageRoute(builder: (context) => DocumentationPage()),
                );
                break;
              case 'Community' || 'Сообщество':
                break;
              case 'Blog' || 'Блог':
                break;
            }
          },
          splashColor: Colors.transparent,
          hoverColor: Colors.transparent,
          highlightColor: Colors.transparent,
          child: RichText(
            text: TextSpan(
              children: [
                TextSpan(
                  text: firstPart,
                  style: TextStyle(
                    fontWeight: FontWeight.bold,
                    color: isActive ? Color(0xFF1A1A1A) : Color(0xFF333333),
                    fontSize: 18,
                    height: 1.0,
                    letterSpacing: 0.5,
                  ),
                ),
                TextSpan(
                  text: secondPart,
                  style: TextStyle(
                    fontWeight: FontWeight.w300,
                    color: isActive ? Color(0xFF333333) : Color(0xFF666666),
                    fontSize: 14,
                    height: 1.0,
                  ),
                ),
              ],
            ),
          ),
        ),
      ),
    ),
  );
}

Widget _getConsoleOutput(CodeExample? example) {
  if (example == null) return Text('No example selected', style: TextStyle(color: Colors.red));
  
  switch (example.name) {
    case 'Fibonacci' || 'Фибоначчи':
      return Text(
        '2971215073',
        style: TextStyle(
          color: Colors.white,
          fontFamily: 'Fira Code',
        ),
      );
    case 'Hello World' || 'Привет, мир':
      return Text(
        'Hello World!!!',
        style: TextStyle(
          color: Colors.white,
          fontFamily: 'Fira Code',
        ),
      );
    case 'Sort' || 'Сортировка':
      return Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            'Before sort: [5, 3, 1, 4, 2]',
            style: TextStyle(
              color: Colors.grey,
              fontFamily: 'Fira Code',
            ),
          ),
          SizedBox(height: 4),
          Text(
            'After sort: [1, 2, 3, 4, 5]',
            style: TextStyle(
              color: Colors.white,
              fontFamily: 'Fira Code',
            ),
          ),
        ],
      );
    case 'General Code' || 'Общий код':
      return Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            'Hellox hz2 World!',
            style: TextStyle(
              color: Colors.white,
              fontFamily: 'Fira Code',
            ),
          ),
          SizedBox(height: 4),
          Text(
            '[main exited with code 4]',
            style: TextStyle(
              color: Colors.grey,
              fontFamily: 'Fira Code',
            ),
          ),
        ],
      );
    case 'Modules' || 'Модули':
      return Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            'a: 5, b: 10',
            style: TextStyle(
              color: Colors.white,
              fontFamily: 'Fira Code',
            ),
          ),
          Text(
            'Sum: 15, Mul: 50',
            style: TextStyle(
              color: Colors.white,
              fontFamily: 'Fira Code',
            ),
          ),
          SizedBox(height: 4),
          Text(
            '[moduleTest exited with code 0]',
            style: TextStyle(
              color: Colors.grey,
              fontFamily: 'Fira Code',
            ),
          ),
        ],
      );
    default:
      return Text(
        'Program exited with code 0',
        style: TextStyle(
          color: Colors.white,
          fontFamily: 'Fira Code',
        ),
      );
  }
}

  String _getExecutionTime(CodeExample? example) {
    if (example == null) return '0';
    
    switch (example.name) {
      case 'Fibonacci' || 'Фибоначчи':
        return '6572';
      case 'Hello World' || 'Привет, мир':
        return '0.73';
      case 'Sort' || 'Сортировка':
        return '0.45';
      case 'General Code' || 'Общий код':
        return '0.83';
      case 'Modules' || 'Модули':
        return '0.78';
      default:
        return '1.23';
    }
  }
}

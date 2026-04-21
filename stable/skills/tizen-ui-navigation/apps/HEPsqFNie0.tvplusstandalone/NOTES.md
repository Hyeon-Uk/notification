# AI Navigation Notes — HEPsqFNie0.tvplusstandalone

## UI Structure

Samsung TV Plus is a web application. `tizen-aurum-cli dump-tree` returns no useful widget hierarchy. The only observability tools are:

- `tizen-context-cli` — returns broadcast JSON with channelName, channelNumber, genre, programName, source ("TVPLUS"), and timing metadata.
- `tizen-context-cli screenshot --capture` — DRM-protected content (music, movies) returns `drm_protected` error for content type; UI capture shows only the thin overlay layer.

The app has no visible channel guide or genre navigation accessible through remote keys. Channel browsing is purely sequential via XF86RaiseChannel / XF86LowerChannel.

The `broadcast.metadata` object includes useful fields:
- `DataValid`: "1" when broadcast info is available, "0" during loading/transition.
- `ChType`: "9" for TV Plus channels.
- `MetaDataSrc`: "TVPlus" confirming the source.
- `GenreId`: genre code (e.g. "G1008" for 뉴스, "G1004" for 예능, "G1001" for 드라마).
- `StartViewTime`: epoch ms when the user started viewing the current channel.

## Tips

- Always check `broadcast.metadata.DataValid == "1"` before trusting channel info — during channel transitions it briefly shows "0".
- Wait 3 seconds after a channel change before reading `tizen-context-cli` — broadcast metadata updates slowly.
- **DataValid="1" with stale channel data**: After a channel change, `tizen-context-cli` may return `DataValid="1"` but still show the *previous* channel's number and name. The channel number is the most reliable freshness indicator — if it doesn't match the target, wait 2–3s and retry. Do not conclude a navigation error based on a single context read.
- The `genre` field from `tizen-context-cli` is often inaccurate — use the channel list below as the authoritative genre reference.
- The app appId `HEPsqFNie0.tvplusstandalone` is stable across sessions — use it to confirm you're in Samsung TV Plus.
- Pressing `back` exits to `com.samsung.tv.csfs` (Home). Allow 1–2 seconds for the Home UI to reload before sending aurum commands.
- Direct channel entry with `key --xf86 <digit>` works for both 3-digit and 4-digit channels. Use 0.3s delay between digits — 0.5s+ is too slow and causes digits to be processed separately.
- **Batch digit key presses** to avoid overhead. Preferred pattern for channel entry:
  ```bash
  tizen-aurum-cli key --xf86 5 && sleep 0.3 && tizen-aurum-cli key --xf86 0 && sleep 0.3 && tizen-aurum-cli key --xf86 3
  ```
  Then wait 3s and call `tizen-context-cli context` to verify the channel change.
- The channel number space is not contiguous — some numbers are skipped.
- Rapid automated channel changes (100+ in sequence) can cause the app to become unresponsive to XF86RaiseChannel/XF86LowerChannel. If stuck, exit with `home`, re-enter TV Plus, and resume.

## Known Issues

- Standard directional keys (up, down, left, right) do not change channels and may trigger unpredictable overlays within the web app.
- DRM-protected content (music videos, some movies) blocks screenshot capture in DRM mode with error "Capture is copy-protected".
- TBM screenshot mode captures only a nearly blank screen with "TV Plus(Standalone)" text — not useful for content identification.
- During the app launch transition (`org.tizen.tv-viewer` → `HEPsqFNie0.tvplusstandalone`), context may show stale data for several seconds.
- The `genre` field in broadcast context is frequently wrong (e.g. 시사/교양 programs reported as 예능, 드라마 reported as 예능). Do not rely on it for genre identification — use the channel list below instead.
- `key <digit>` (without `--xf86`) sends the wrong keycode — it can exit the app instead of entering a channel number. Always use `key --xf86 <digit>`.
- After first entering TV Plus, `tizen-context-cli` may return `DataValid: "0"` with empty broadcast fields. Send one channel change (`XF86RaiseChannel` or `XF86LowerChannel`) and navigate back to trigger valid context.

## Channel List (Korea)

> **Last scanned: 2026-04-17.** This list is mutable — Samsung may add, remove, or renumber channels at any time. Update this list when you discover a channel has changed.

Total channels: 179

### 예능 (63)

| Channel | Name |
|---|---|
| 507 | 지구마불 세계여행 |
| 508 | 쯔양 |
| 527 | SBS No.1 뉴스라이브 |
| 539 | JTBC |
| 540 | JTBC2 |
| 541 | MBC 무한도전 |
| 542 | SBS 런닝맨 |
| 545 | KBS 1박2일 시즌1 |
| 546 | tvN 삼시세끼 어촌편 5 |
| 547 | 빠니보틀 |
| 548 | tvN 삼시세끼 고창편 |
| 549 | tvN 신서유기 7~8 이어보기 |
| 550 | MBC 지붕뚫고 하이킥 |
| 551 | MBC 거침없이 하이킥 |
| 552 | tvN 대탈출 전 시즌 |
| 554 | SBS 정글의 법칙 |
| 555 | SBS 순풍산부인과 |
| 556 | MBC 심야괴담회 |
| 559 | JTBC 톡파원 25시 |
| 560 | MBC 나혼자산다 |
| 561 | MBC 구해줘홈즈 |
| 562 | JTBC 한문철의 블랙박스리뷰 |
| 565 | MBC 안싸우면 다행이야 |
| 566 | MBC 라디오스타 |
| 569 | 나는 솔로 |
| 570 | MBC 전지적 참견 시점 |
| 571 | MBC 놀면 뭐하니 |
| 572 | MBC HOT CLIP 예능 |
| 573 | SBS 불타는 청춘 |
| 574 | JTBC 냉장고를 부탁해 2 |
| 575 | JTBC 이혼숙려캠프 |
| 576 | TV조선 미스터로또 |
| 577 | JTBC 효리네 민박1&2 |
| 578 | TV조선 사랑의 콜센타 |
| 579 | JTBC 냉장고를 부탁해 |
| 582 | KBS HOT CLIP 예능 |
| 583 | MBC 오은영리포트 결혼지옥 |
| 584 | SBS 미운 우리 새끼 |
| 585 | SBS 백종원의 골목식당 |
| 589 | SBS 신발 벗고 돌싱포맨 |
| 590 | JTBC 크라임씬3 |
| 591 | KBS 개는 훌륭하다 |
| 592 | 비보티비 |
| 598 | SBS 패밀리가 떴다 |
| 599 | 바오패밀리 |
| 600 | 할명수 |
| 603 | 살롱드립 |
| 605 | TV조선 생존왕 |
| 607 | SBS HOT CLIP 예능 |
| 622 | tvN 슬기로운 의사생활 1~2 이어보기 |
| 623 | tvN 응답하라 전 시즌 모아보기 |
| 672 | 서울경제TV |
| 674 | 연합뉴스경제TV |
| 677 | MTN 머니투데이방송 |
| 681 | SBS TV 동물농장 |
| 685 | SBS 궁금한 이야기 Y |
| 686 | SBS 그것이 알고싶다 |
| 687 | SBS 순간포착 세상에 이런일이 |
| 688 | SBS 생활의 달인 |
| 745 | 씨네21+ |
| 761 | 우리의식탁 |
| 791 | Fireplace 4K |
| 1009 | (Test) Samsung Television Network |

### 드라마 (32)

| Channel | Name |
|---|---|
| 509 | KBS 겨울연가 (Remastered) |
| 510 | SBS 토마토 (Remastered) |
| 511 | MBC 로망스 (Remastered) |
| 512 | KBS 최고다 이순신 |
| 520 | Genie 봄맞이 로맨스 특집 |
| 538 | JTBC4 |
| 621 | SBS 모범택시 |
| 625 | SBS 야인시대 |
| 626 | KBS 구르미 그린 달빛 |
| 628 | KBS 신사와 아가씨 |
| 629 | KBS 태조 왕건 |
| 631 | <신병> 정주행 |
| 632 | JTBC 재벌집 막내아들 |
| 634 | 나의 해리에게 |
| 636 | SBS HOT CLIP 드라마 |
| 637 | SBS 내 남자의 여자 |
| 638 | 행복배틀 |
| 640 | MBC 밤에 피는 꽃 |
| 643 | 야한 사진관 |
| 644 | SBS 올인 |
| 646 | MBC HOT CLIP 드라마 |
| 650 | 마당이 있는 집 |
| 651 | 가우스전자 |
| 654 | 구필수는 없다 |
| 655 | JTBC 라이프 |
| 656 | 나미브 |
| 658 | 낮에 뜨는 달 |
| 659 | 라이딩 인생 |
| 1005 | KBS 가을동화 (Remastered) |
| 1006 | MBC 다모 (Remastered) |
| 1007 | SBS 명랑소녀 성공기 (Remastered) |
| 1012 | (Test) 리마스터링 타이틀 개편 |

### 스포츠 (12)

| Channel | Name |
|---|---|
| 811 | FIFA+ |
| 812 | PGA Tour |
| 813 | OGN 스타리그 |
| 815 | 초록뱀미디어 K-STAR 골프 |
| 816 | World Billiards TV |
| 817 | tvN SPORTS |
| 818 | JTBC 골프 |
| 819 | 요가소년 |
| 820 | 힙으뜸 |
| 821 | JTBC 뭉쳐야 찬다3 |
| 822 | 스크린골프존 |
| 823 | JTBC 최강야구 |

### 뉴스 (7)

| Channel | Name |
|---|---|
| 501 | YTN |
| 502 | 연합뉴스TV |
| 526 | KBS 뉴스24 |
| 529 | TV조선 |
| 671 | JTBC 뉴스 |
| 793 | Bloomberg TV+ UHD |
| 794 | Bloomberg Originals |

### 리얼리티 (6)

| Channel | Name |
|---|---|
| 503 | tvN 보검매직컬 |
| 505 | tvN 장사천재 백사장 1~2 이어보기 |
| 516 | tvN 진짜 괜찮은 사람 |
| 544 | tvN 콩콩팥팥 & 콩콩밥밥 |
| 564 | tvN 어쩌다 사장 전 시즌 이어보기 |
| 606 | tvN STORY |

### 음악 (5)

| Channel | Name |
|---|---|
| 721 | essential; by Bugs |
| 722 | JTBC 비긴어게인 - 오픈마이크 |
| 723 | M2 |
| 726 | KBS HOT CLIP 음악 |
| 729 | RTS 테스트 채널 |

### 버라이어티 (4)

| Channel | Name |
|---|---|
| 515 | tvN 서진이네 1~2 이어보기 |
| 543 | tvN 뿅뿅 지구오락실 전 시즌 모아보기 |
| 580 | tvN 유 퀴즈 온 더 블럭 HOT CLIP |
| 581 | tvN 놀라운 토요일 하이라이트 |

### 코미디 (4)

| Channel | Name |
|---|---|
| 504 | tvN 철인왕후 |
| 518 | tvN 그놈은 흑염룡 |
| 519 | tvN 금주를 부탁해 |
| 798 | 코난 오브라이언 쇼 |

### 어린이 (3)

| Channel | Name |
|---|---|
| 706 | 투니버스 흔한남매의 안흔한일기 |
| 710 | 뽀요TV |
| 712 | Kids time |

### 교육 (2)

| Channel | Name |
|---|---|
| 684 | tvN 벌거벗은 세계사 요약.zip |
| 792 | TED |

### 로맨스 (2)

| Channel | Name |
|---|---|
| 649 | tvN 인급동 |
| 742 | CJ ENM MOVIE |

### 쇼핑 (2)

| Channel | Name |
|---|---|
| 763 | 현대홈쇼핑 |
| 764 | 현대홈쇼핑+Shop |

### 요리 (2)

| Channel | Name |
|---|---|
| 530 | MBN |
| 609 | 더라이프2 |

### 카툰 (2)

| Channel | Name |
|---|---|
| 701 | 투니버스 짱구는 못말려 |
| 703 | 투니버스 명탐정 코난 (자막) |

### Financial (1)

| Channel | Name |
|---|---|
| 675 | 매일경제TV |

### Go (1)

| Channel | Name |
|---|---|
| 814 | K바둑 |

### 다큐멘터리 (1)

| Channel | Name |
|---|---|
| 765 | SAMSUNG UHD |

### 애니메이션 (1)

| Channel | Name |
|---|---|
| 704 | 캐리TV |

### 액션 (1)

| Channel | Name |
|---|---|
| 743 | Genie Movie |

### 여행 (1)

| Channel | Name |
|---|---|
| 608 | JTBC 캠핑클럽 |

### 커뮤니티 (1)

| Channel | Name |
|---|---|
| 690 | 아리랑TV |

### 콘서트 (1)

| Channel | Name |
|---|---|
| 506 | Mnet 월드 오브 스트릿 우먼 파이터 |

### 토크쇼 (1)

| Channel | Name |
|---|---|
| 557 | tvN STORY 김창옥쇼 3 |

### 판타지 (1)

| Channel | Name |
|---|---|
| 645 | tvN 환혼 4K |

### (미분류) (23)

| Channel | Name |
|---|---|
| 513 | EBS 극한 직업 |
| 553 | 맛있는 녀석들 |
| 558 | 채널A 나만 믿고 따라와 도시어부 |
| 563 | 용감한 형사들 |
| 586 | KBS Joy 연애의 참견 |
| 594 | iHQ 돈쭐내러 왔습니다 |
| 601 | MBN 속풀이쇼 동치미 |
| 604 | KBS Joy 무엇이든 물어보살 |
| 624 | 고독한 미식가 |
| 642 | MBC every1 별순검 |
| 653 | TV조선 나의 해피엔드 |
| 673 | MBN 뉴스 |
| 676 | 한국경제TV |
| 683 | MBN 나는 자연인이다 |
| 689 | MBN 휴먼다큐 사노라면 |
| 702 | 베베핀 |
| 705 | Animax 요리왕 비룡 더 마스터 |
| 708 | ch.핑크퐁 |
| 711 | 코코비TV |
| 724 | K-ASMR |
| 744 | PLAYGRAM MOVIES |
| 795 | 글로벌 뉴스 by LeadStory |
| 898 | (Test) KBS CMAF |

